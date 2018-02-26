#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "resources.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "assetimporter.h"
#include "os/filesystem.h"
#include "internal/file/image.h"
#include "internal/world/worldinternal.h"
#include "internal/base/animationinternal.h"

static glm::mat4& AIMaterixToGLM(glm::mat4& answer, const aiMatrix4x4& mat) {
	answer = glm::mat4(
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
	);

	return answer;
}

static glm::quat& AIQuaternionToGLM(glm::quat& answer, const aiQuaternion& quaternion) {
	answer = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
	return answer;
}

static void DecomposeAIMatrix(glm::vec3& translation, glm::quat& rotation, glm::vec3& scale, const aiMatrix4x4& mat) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::mat4 transformation;
	AIMaterixToGLM(transformation, mat);
	glm::decompose(transformation, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);
}

static glm::vec3& AIVector3ToGLM(glm::vec3& answer, const aiVector3D& vec) {
	answer = glm::vec3(vec.x, vec.y, vec.z);
	return answer;
}

AssetImporter::MaterialAttribute::MaterialAttribute()
	: twoSided(false), gloss(20), mainColor(1), name(UNNAMED_MATERIAL) {
}

Entity AssetImporter::Import(const std::string& path) {
	Entity entity = NewEntity();
	ImportTo(entity, path);
	return entity;
}

bool AssetImporter::ImportTo(Entity entity, const std::string& path) {
	if (!entity) {
		Debug::LogError("invalid entity");
		return false;
	}

	Debug::StartSample();
	Assimp::Importer importer;
	if (!Initialize(path, importer)) {
		return false;
	}

	Debug::Output("[init assimp]\t%.3f\n", Debug::EndSample());

	Debug::StartSample();

	TriangleBase* subMeshBases = nullptr;
	MeshAttribute attribute{ MeshTopologyTriangles };

	Material* materials = nullptr;

	if (scene_->mNumMaterials > 0) {
		materials = MEMORY_CREATE_ARRAY(Material, scene_->mNumMaterials);
		if (!ReadMaterials(materials)) {
			Debug::LogError("failed to load materials for %s.", path.c_str());
		}
	}

	if (scene_->mNumMeshes > 0) {
		subMeshBases = MEMORY_CREATE_ARRAY(TriangleBase, scene_->mNumMeshes);
		if (!ReadAttribute(attribute, subMeshBases)) {
			Debug::LogError("failed to load meshes for %s.", path.c_str());
		}
	}

	Mesh surface = NewMesh();
	surface->SetAttribute(attribute);

	Debug::Output("[read attributes]\t%.3f\n", Debug::EndSample());

	Debug::StartSample();
	
	ReadNodeTo(entity, scene_->mRootNode, surface, subMeshBases, materials);
	ReadChildren(entity, scene_->mRootNode, surface, subMeshBases, materials);

	Debug::Output("[read hierarchy]\t%.3f\n", Debug::EndSample());

	MEMORY_RELEASE_ARRAY(materials);
	MEMORY_RELEASE_ARRAY(subMeshBases);

	Animation animation;
	if (ReadAnimation(animation)) {
		entity->SetAnimation(animation);
	}

	return true;
}

bool AssetImporter::Initialize(const std::string& path, Assimp::Importer &importer) {
	uint flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
		| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs
		| aiProcess_OptimizeMeshes/* | aiProcess_OptimizeGraph*/ | aiProcess_RemoveRedundantMaterials;

	if (FileSystem::GetExtension(path) == ".fbx") {
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	}

	std::string fpath = Resources::GetRootDirectory() + path;

	const aiScene* scene = importer.ReadFile(fpath.c_str(), flags);
	if (scene == nullptr) {
		Debug::LogError("failed to read file %s: %s", fpath.c_str(), importer.GetErrorString());
		return false;
	}

	Clear();

	path_ = fpath;
	scene_ = scene;

	return true;
}

void AssetImporter::Clear() {
	path_.clear();
	scene_ = nullptr;
	skeleton_.reset();
	textures_.clear();
	animation_.reset();
}

Entity AssetImporter::ReadHierarchy(Entity parent, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials) {
	Entity entity = NewEntity();
	entity->GetTransform()->SetParent(parent->GetTransform());

	ReadNodeTo(entity, node, surface, bases, materials);
	ReadChildren(entity, node, surface, bases, materials);

	return entity;
}

void AssetImporter::ReadNodeTo(Entity entity, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials) {
	entity->SetName(node->mName.C_Str());

	glm::vec3 translation, scale;
	glm::quat rotation;
	DecomposeAIMatrix(translation, rotation, scale, node->mTransformation);

	entity->GetTransform()->SetLocalScale(scale);
	entity->GetTransform()->SetLocalRotation(rotation);
	entity->GetTransform()->SetLocalPosition(translation);

	ReadComponents(entity, node, surface, bases, materials);
}

void AssetImporter::ReadComponents(Entity entity, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials) {
	Renderer renderer = nullptr;
	if (scene_->mNumAnimations == 0) {
		renderer = NewMeshRenderer();
	}
	else {
		renderer = NewSkinnedMeshRenderer();
		dsp_cast<SkinnedMeshRenderer>(renderer)->SetSkeleton(skeleton_);
	}

	Mesh mesh = NewMesh();
	mesh->ShareStorage(surface);

	for (int i = 0; i < node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		SubMesh subMesh = NewSubMesh();
		subMesh->SetTriangles(bases[meshIndex]);
		mesh->AddSubMesh(subMesh);

		uint materialIndex = scene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(materials[materialIndex]);
		}
	}

	entity->SetMesh(mesh);
	entity->SetRenderer(renderer);
}

void AssetImporter::ReadChildren(Entity entity, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials) {
	for (int i = 0; i < node->mNumChildren; ++i) {
		ReadHierarchy(entity, node->mChildren[i], surface, bases, materials);
	}
}

void AssetImporter::ReserveMemory(MeshAttribute& attribute) {
	int indexCount = 0, vertexCount = 0;
	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		indexCount += scene_->mMeshes[i]->mNumFaces * 3;
		vertexCount += scene_->mMeshes[i]->mNumVertices;
	}

	attribute.positions.reserve(vertexCount);
	attribute.normals.reserve(vertexCount);
	attribute.texCoords.reserve(vertexCount);
	attribute.tangents.reserve(vertexCount);
	attribute.indexes.reserve(indexCount);
	attribute.blendAttrs.resize(vertexCount);
}

bool AssetImporter::ReadAttribute(MeshAttribute& attribute, TriangleBase* bases) {
	ReserveMemory(attribute);

	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		bases[i].baseIndex = attribute.indexes.size();
		bases[i].baseVertex = attribute.positions.size();
		ReadAttributeAt(i, attribute, bases);
		bases[i].indexCount = scene_->mMeshes[i]->mNumFaces * 3;
	}

	return true;
}

bool AssetImporter::ReadAttributeAt(int meshIndex, MeshAttribute& attribute, TriangleBase* bases) {
	ReadVertexAttribute(meshIndex, attribute);
	ReadBoneAttribute(meshIndex, attribute, bases);

	return true;
}

void AssetImporter::ReadVertexAttribute(int meshIndex, MeshAttribute& attribute) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];

	// TODO: multiple texture coords?
	for (int i = 1; i < AI_MAX_NUMBER_OF_COLOR_SETS; ++i) {
		if (aimesh->HasTextureCoords(i)) {
			Debug::LogWarning("multiple texture coordinates");
		}
	}

	bool logged = false;
	const aiVector3D zero(0);
	for (uint i = 0; i < aimesh->mNumVertices; ++i) {
		const aiVector3D* pos = &aimesh->mVertices[i];
		const aiVector3D* normal = &aimesh->mNormals[i];

		// TODO:
		if (aimesh->GetNumUVChannels() != 1 && !logged) {
			logged = true;
			Debug::LogWarning("this mesh contains %d uv channel(s).", aimesh->GetNumUVChannels());
		}

		const aiVector3D* texCoord = aimesh->HasTextureCoords(0) ? &(aimesh->mTextureCoords[0][i]) : &zero;
		const aiVector3D* tangent = (aimesh->mTangents != nullptr) ? &aimesh->mTangents[i] : &zero;

		attribute.positions.push_back(glm::vec3(pos->x, pos->y, pos->z));
		attribute.normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
		attribute.texCoords.push_back(glm::vec2(texCoord->x, texCoord->y));
		attribute.tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));
	}

	for (uint i = 0; i < aimesh->mNumFaces; ++i) {
		const aiFace& face = aimesh->mFaces[i];
		if (face.mNumIndices != 3) {
			Debug::LogError("invalid index count");
			continue;
		}

		attribute.indexes.push_back(face.mIndices[0]);
		attribute.indexes.push_back(face.mIndices[1]);
		attribute.indexes.push_back(face.mIndices[2]);
	}
}

void AssetImporter::ReadBoneAttribute(int meshIndex, MeshAttribute& attribute, TriangleBase* bases) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];
	for (int i = 0; i < aimesh->mNumBones; ++i) {
		if (!skeleton_) { skeleton_ = NewSkeleton(); }
		std::string name(aimesh->mBones[i]->mName.data);

		int index = skeleton_->GetBoneIndex(name);
		if (index < 0) {
			SkeletonBone bone;
			bone.name = name;
			AIMaterixToGLM(bone.localToBoneSpaceMatrix, aimesh->mBones[i]->mOffsetMatrix);
			index = skeleton_->GetBoneCount();
			skeleton_->AddBone(bone);
		}

		for (int j = 0; j < aimesh->mBones[i]->mNumWeights; ++j) {
			uint vertexID = bases[meshIndex].baseVertex + aimesh->mBones[i]->mWeights[j].mVertexId;

			float weight = aimesh->mBones[i]->mWeights[j].mWeight;
			for (int k = 0; k < BlendAttribute::Quality; ++k) {
				if (Math::Approximately(attribute.blendAttrs[vertexID].weights[k])) {
					attribute.blendAttrs[vertexID].indexes[k] = index;
					attribute.blendAttrs[vertexID].weights[k] = weight;
					break;
				}
			}
		}
	}
}

bool AssetImporter::ReadMaterials(Material* materials) {
	for (int i = 0; i < scene_->mNumMaterials; ++i) {
		MaterialAttribute surface;
		ReadMaterialAttribute(surface, scene_->mMaterials[i]);
		
		Material material = NewMaterial();
		ReadMaterial(material, surface);

		materials[i] = material;
	}

	return true;
}

bool AssetImporter::ReadMaterial(Material material, const MaterialAttribute& surface) {
	std::string shaderName = "lit_texture";
	if (scene_->mNumAnimations != 0) {
		shaderName = "lit_animated_texture";
	}

	// TODO: two sided.

	Shader shader = Resources::FindShader("buildin/shaders/" + shaderName);
	material->SetShader(shader);

	material->SetFloat(Variables::gloss, surface.gloss);

	material->SetColor4(Variables::mainColor, surface.mainColor);
	material->SetColor3(Variables::specularColor, surface.specularColor);
	material->SetColor3(Variables::emissiveColor, surface.emissiveColor);

	material->SetName(surface.name);

	if (surface.mainTexture) {
		material->SetTexture(Variables::mainTexture, surface.mainTexture);
	}

	if (surface.bumpTexture) {
		material->SetTexture(Variables::bumpTexture, surface.bumpTexture);
	}

	if (surface.specularTexture) {
		material->SetTexture(Variables::specularTexture, surface.specularTexture);
	}

	if (surface.emissiveTexture) {
		material->SetTexture(Variables::emissiveTexture, surface.emissiveTexture);
	}

	if (surface.lightmapTexture) {
		material->SetTexture(Variables::lightmapTexture, surface.lightmapTexture);
	}

	return true;
}

void AssetImporter::ReadMaterialAttribute(MaterialAttribute& surface, aiMaterial* material) {
	int aint;
	float afloat;
	aiString astring;
	aiColor3D acolor;

	if (material->Get(AI_MATKEY_NAME, astring) == AI_SUCCESS) {
		surface.name = FileSystem::GetFileName(astring.C_Str());
	}
	
	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), astring) == AI_SUCCESS) {
		surface.mainTexture = GetTexture(FileSystem::GetFileName(astring.C_Str()));
	}

	if (!surface.mainTexture) {
		surface.mainTexture = Resources::GetWhiteTexture();
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), astring) == AI_SUCCESS) {
		surface.bumpTexture = GetTexture(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), astring) == AI_SUCCESS) {
		surface.specularTexture = GetTexture(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), astring) == AI_SUCCESS) {
		surface.lightmapTexture = GetTexture(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), astring) == AI_SUCCESS) {
		surface.emissiveTexture = GetTexture(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_OPACITY, afloat) == AI_SUCCESS) {
		if (Math::Approximately(afloat)) { afloat = 1; }
		surface.mainColor.a = afloat;
	}

	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, acolor) == AI_SUCCESS) {
		surface.mainColor = glm::vec4(acolor.r, acolor.g, acolor.b, surface.mainColor.a);
	}

	if (material->Get(AI_MATKEY_COLOR_SPECULAR, acolor) == AI_SUCCESS) {
		surface.specularColor = glm::vec3(acolor.r, acolor.g, acolor.b);
	}

	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, acolor) == AI_SUCCESS) {
		surface.emissiveColor = glm::vec3(acolor.r, acolor.g, acolor.b);
	}

	if (material->Get(AI_MATKEY_SHININESS, afloat) == AI_SUCCESS) {
		surface.gloss = afloat;
	}

	if (material->Get(AI_MATKEY_TWOSIDED, aint) == AI_SUCCESS) {
		surface.twoSided = !!aint;
	}
}

bool AssetImporter::ReadAnimation(Animation& animation) {
	if (scene_->mNumAnimations == 0) {
		return true;
	}
	
	animation = NewAnimation();

	glm::mat4 rootTransform;
	animation->SetRootTransform(AIMaterixToGLM(rootTransform, scene_->mRootNode->mTransformation.Inverse()));

	const char* defaultClipName = nullptr;
	for (int i = 0; i < scene_->mNumAnimations; ++i) {
		aiAnimation* anim = scene_->mAnimations[i];
		std::string name = anim->mName.C_Str();

		AnimationClip clip = NewAnimationClip();
		if (defaultClipName == nullptr) {
			defaultClipName = anim->mName.C_Str();
		}

		ReadAnimationClip(anim, clip);
		animation->AddClip(name, clip);
	}

	animation->SetSkeleton(skeleton_);
	animation->Play(defaultClipName);

	return true;
}

void AssetImporter::ReadAnimationClip(const aiAnimation* anim, AnimationClip clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapModeLoop);
	ReadAnimationNode(anim, scene_->mRootNode, nullptr);
}

void AssetImporter::ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
	const aiNodeAnim* channel = FindChannel(anim, paiNode->mName.C_Str());

	AnimationCurve curve;
	AnimationKeys keys = NewAnimationKeys();
	if (channel != nullptr) {
		for (int i = 0; i < channel->mNumPositionKeys; ++i) {
			const aiVectorKey& key = channel->mPositionKeys[i];
			glm::vec3 position;
			keys->AddVector3(FrameKeyPosition, (float)key.mTime, AIVector3ToGLM(position, key.mValue));
		}

		for (int i = 0; i < channel->mNumRotationKeys; ++i) {
			const aiQuatKey& key = channel->mRotationKeys[i];
			glm::quat rotation;
			keys->AddQuaternion(FrameKeyRotation, (float)key.mTime, AIQuaternionToGLM(rotation, key.mValue));
		}

		for (int i = 0; i < channel->mNumScalingKeys; ++i) {
			const aiVectorKey& key = channel->mScalingKeys[i];
			glm::vec3 scale;
			keys->AddVector3(FrameKeyScale, (float)key.mTime, AIVector3ToGLM(scale, key.mValue));
		}

		std::vector<AnimationFrame> keyframes;
		keys->ToKeyframes(keyframes);

		curve = NewAnimationCurve();
		curve->SetKeyframes(keyframes);
	}

	glm::mat4 matrix;
	SkeletonNode* child = skeleton_->CreateNode(paiNode->mName.C_Str(), AIMaterixToGLM(matrix, paiNode->mTransformation), curve);
	skeleton_->AddNode(pskNode, child);

	for (int i = 0; i < paiNode->mNumChildren; ++i) {
		ReadAnimationNode(anim, paiNode->mChildren[i], child);
	}
}

const aiNodeAnim* AssetImporter::FindChannel(const aiAnimation* anim, const char* name) {
	for (int i = 0; i < anim->mNumChannels; ++i) {
		if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), name) == 0) {
			return anim->mChannels[i];
		}
	}

	return nullptr;
}

Texture AssetImporter::GetTexture(const std::string& name) {
	TextureContainer::iterator pos = textures_.find(name);
	if (pos != textures_.end()) {
		return pos->second;
	}

	Texture texture = nullptr;
	if (String::StartsWith(name, "*")) {
		texture = ReadEmbeddedTexture(String::ToInteger(name.substr(1)));
	}
	else {
		texture = ReadExternalTexture(name);
	}

	textures_.insert(std::make_pair(name, texture));
	return texture;
}

Texture AssetImporter::ReadExternalTexture(const std::string& name) {
	Texture2D texture = NewTexture2D();
	if (!texture->Load("textures/" + name)) {
		return nullptr;
	}

	return texture;
}

Texture AssetImporter::ReadEmbeddedTexture(uint index) {
	if (index >= scene_->mNumTextures) {
		Debug::LogError("embedded texture index out of range");
		return nullptr;
	}

	Texture2D texture = NewTexture2D();
	aiTexture* aitex = scene_->mTextures[index];
	if (aitex->mHeight == 0) {
		TexelMap texelMap;
		if (!ImageCodec::Decode(texelMap, aitex->pcData, aitex->mWidth)) {
			return nullptr;
		}

		texture->Load(texelMap.textureFormat, &texelMap.data[0], texelMap.format, texelMap.width, texelMap.height);
	}
	else {
		texture->Load(TextureFormatRgba, aitex->pcData, ColorStreamFormatArgb, aitex->mWidth, aitex->mHeight);
	}

	return texture;
}
