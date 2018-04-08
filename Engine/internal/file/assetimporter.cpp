#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "texture.h"
#include "resources.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "assetimporter.h"
#include "os/filesystem.h"
#include "internal/file/image.h"
#include "internal/world/worldinternal.h"
#include "internal/base/animationinternal.h"

#include <OpenThreads/Thread>

struct MaterialAttribute {
	MaterialAttribute();

	Material material;

	std::string name;
	std::string shaderName;

	float gloss;
	bool twoSided;

	glm::vec4 mainColor;
	glm::vec3 specularColor;
	glm::vec3 emissiveColor;

	TexelMap* mainTexels;
	TexelMap* bumpTexels;
	TexelMap* specularTexels;
	TexelMap* emissiveTexels;
	TexelMap* lightmapTexels;
};

struct AssetData {
	MeshAttribute meshAttribute;
	std::vector<MaterialAttribute> materialAttributes;
};

inline std::string ParseThreadError(int err) {
	if (err == -1) {
		return "errno = " + std::to_string(errno);
	}

	return "error = " + std::to_string(err);
}

class AssetDataLoader : public OpenThreads::Thread {
public:
	virtual void run() {
		Assimp::Importer importer;
		if (!Initialize(importer)) {
			return;
		}

		SubMesh* subMeshes = nullptr;
		MeshAttribute attribute{ MeshTopologyTriangles };

		if (scene_->mNumMaterials > 0) {
			data_.materialAttributes.resize(scene_->mNumMaterials);
			ReadMaterials();
		}

		if (scene_->mNumMeshes > 0) {
			subMeshes = MEMORY_CREATE_ARRAY(SubMesh, scene_->mNumMeshes);
			if (!ReadAttribute(attribute, subMeshes)) {
				Debug::LogError("failed to load meshes for %s.", path_.c_str());
			}
		}

		Mesh surface = NewMesh();
		
		surface->CreateStorage();

		//surface->SetAttribute(attribute);

		//Debug::Output("[read attributes]\t%.3f\n", Debug::EndSample());

		//Debug::StartSample();

		ReadNodeTo(target_, scene_->mRootNode, surface, subMeshes);
		ReadChildren(target_, scene_->mRootNode, surface, subMeshes);

		//Debug::Output("[read hierarchy]\t%.3f\n", Debug::EndSample());

		MEMORY_RELEASE_ARRAY(subMeshes);

		Animation animation;
		if (ReadAnimation(animation)) {
			target_->SetAnimation(animation);
		}
	}

public:
	void SetTarget(const std::string& path, Entity entity) {
		path_ = path;
		target_ = entity;
	}

	AssetData& GetLoadedData() {
		return data_;
	}

private:
	void Clear();
	bool Initialize(Assimp::Importer& importer);
	void UpdateMaterial(MaterialAttribute& attribute);

	Entity ReadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes);

	void ReadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);
	void ReadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);
	void ReadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);

	bool ReadAttribute(MeshAttribute& attribute, SubMesh* subMeshes);
	void ReserveMemory(MeshAttribute& attribute);
	bool ReadAttributeAt(int index, MeshAttribute& attribute, SubMesh* subMeshes);

	void ReadVertexAttribute(int meshIndex, MeshAttribute& attribute);
	void ReadBoneAttribute(int meshIndex, MeshAttribute& attribute, SubMesh* subMeshes);

	void ReadMaterials();

	void ReadMaterialAttribute(MaterialAttribute& attribute, aiMaterial* material);

	bool ReadAnimation(Animation& animation);
	void ReadAnimationClip(const aiAnimation* anim, AnimationClip clip);
	void ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim * FindChannel(const aiAnimation* anim, const char* name);

	TexelMap* ReadTexels(const std::string& name);

	bool ReadEmbeddedTexels(TexelMap& texelMap, uint index);
	bool ReadExternalTexels(TexelMap& texelMap, const std::string& name);

private:
	Entity target_;
	AssetData data_;

	Skeleton skeleton_;
	std::string path_;
	const aiScene* scene_;
	Animation animation_;

	typedef std::map<std::string, TexelMap*> TexelMapContainer;
	TexelMapContainer texelMapContainer_;
};

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

MaterialAttribute::MaterialAttribute()
	: twoSided(false), gloss(20), mainColor(1), name(UNNAMED_MATERIAL) {
	material = NewMaterial();
	mainTexels = bumpTexels = specularTexels = emissiveTexels = lightmapTexels = nullptr;
}

AssetImporter::AssetImporter() {
	loader_ = MEMORY_CREATE(AssetDataLoader);
}

AssetImporter::~AssetImporter() {
	loader_->cancel();
	MEMORY_RELEASE(loader_);
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

	if (loader_->isRunning()) {
		Debug::LogError("asset importer is running");
		return false;
	}

	loader_->SetTarget(path, entity);

	int err = loader_->start();
	if (err != 0) {
		Debug::LogError("failed to start thread: %s", ParseThreadError(err).c_str());
		return false;
	}

	return true;
}

bool AssetDataLoader::Initialize(Assimp::Importer &importer) {
	uint flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices 
		| aiProcess_ImproveCacheLocality | aiProcess_FindInstances | aiProcess_GenSmoothNormals 
		| aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_OptimizeMeshes /*| aiProcess_OptimizeGraph*/ 
		| aiProcess_RemoveRedundantMaterials;

	if (FileSystem::GetExtension(path_) == ".fbx") {
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	}

	std::string fpath = Resources::GetRootDirectory() + path_;

	const aiScene* scene = importer.ReadFile(fpath.c_str(), flags);
	if (scene == nullptr) {
		Debug::LogError("failed to read file %s: %s", path_.c_str(), importer.GetErrorString());
		return false;
	}

	Clear();
	path_ = fpath;
	scene_ = scene;

	return true;
}

void AssetDataLoader::Clear() {
	path_.clear();

	data_.meshAttribute = MeshAttribute();
	data_.materialAttributes.clear();

	scene_ = nullptr;
	skeleton_.reset();

	for (TexelMapContainer::iterator ite = texelMapContainer_.begin(); ite != texelMapContainer_.end(); ++ite) {
		MEMORY_RELEASE(ite->second);
	}
	texelMapContainer_.clear();

	animation_.reset();
}

Entity AssetDataLoader::ReadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	Entity entity = NewEntity();
	entity->GetTransform()->SetParent(parent->GetTransform());

	ReadNodeTo(entity, node, surface, subMeshes);
	ReadChildren(entity, node, surface, subMeshes);

	return entity;
}

void AssetDataLoader::ReadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	entity->SetName(node->mName.C_Str());

	glm::vec3 translation, scale;
	glm::quat rotation;
	DecomposeAIMatrix(translation, rotation, scale, node->mTransformation);

	entity->GetTransform()->SetLocalScale(scale);
	entity->GetTransform()->SetLocalRotation(rotation);
	entity->GetTransform()->SetLocalPosition(translation);

	ReadComponents(entity, node, surface, subMeshes);
}

void AssetDataLoader::ReadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
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
		mesh->AddSubMesh(subMeshes[meshIndex]);

		uint materialIndex = scene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(data_.materialAttributes[materialIndex].material);
		}
	}

	entity->SetMesh(mesh);
	entity->SetRenderer(renderer);
}

void AssetDataLoader::ReadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	for (int i = 0; i < node->mNumChildren; ++i) {
		ReadHierarchy(entity, node->mChildren[i], surface, subMeshes);
	}
}

void AssetDataLoader::ReserveMemory(MeshAttribute& attribute) {
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

bool AssetDataLoader::ReadAttribute(MeshAttribute& attribute, SubMesh* subMeshes) {
	ReserveMemory(attribute);

	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		subMeshes[i] = NewSubMesh();
		TriangleBias bias{ 
			scene_->mMeshes[i]->mNumFaces * 3, attribute.indexes.size(), attribute.positions.size()
		};
		subMeshes[i]->SetTriangleBias(bias);
		ReadAttributeAt(i, attribute, subMeshes);
	}

	return true;
}

bool AssetDataLoader::ReadAttributeAt(int meshIndex, MeshAttribute& attribute, SubMesh* subMeshes) {
	ReadVertexAttribute(meshIndex, attribute);
	ReadBoneAttribute(meshIndex, attribute, subMeshes);

	return true;
}

void AssetDataLoader::ReadVertexAttribute(int meshIndex, MeshAttribute& attribute) {
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

void AssetDataLoader::ReadBoneAttribute(int meshIndex, MeshAttribute& attribute, SubMesh* subMeshes) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];
	for (int i = 0; i < aimesh->mNumBones; ++i) {
		if (!skeleton_) { skeleton_ = NewSkeleton(); }
		std::string name(aimesh->mBones[i]->mName.data);

		int index = skeleton_->GetBoneIndex(name);
		if (index < 0) {
			SkeletonBone bone;
			bone.name = name;
			AIMaterixToGLM(bone.localToBoneMatrix, aimesh->mBones[i]->mOffsetMatrix);
			index = skeleton_->GetBoneCount();
			skeleton_->AddBone(bone);
		}

		for (int j = 0; j < aimesh->mBones[i]->mNumWeights; ++j) {
			uint vertexID = subMeshes[meshIndex]->GetTriangleBias().baseVertex + aimesh->mBones[i]->mWeights[j].mVertexId;

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

void AssetDataLoader::ReadMaterials() {
	for (int i = 0; i < scene_->mNumMaterials; ++i) {
		ReadMaterialAttribute(data_.materialAttributes[i], scene_->mMaterials[i]);
	}
}

inline Texture2D CreateTexture2D(const TexelMap* texelMap) {
	Texture2D texture = NewTexture2D();
	if (!texture->Load(texelMap->textureFormat, &texelMap->data[0], texelMap->format, texelMap->width, texelMap->height, false)) {
		return nullptr;
	}

	return texture;
}

void AssetDataLoader::UpdateMaterial(MaterialAttribute& materialAttribute) {
	// TODO: two sided.
	Material material = materialAttribute.material;

	Shader shader = Resources::FindShader("buildin/shaders/" + materialAttribute.shaderName);
	material->SetShader(shader);

	material->SetFloat(Variables::gloss, materialAttribute.gloss);

	material->SetColor4(Variables::mainColor, materialAttribute.mainColor);
	material->SetColor3(Variables::specularColor, materialAttribute.specularColor);
	material->SetColor3(Variables::emissiveColor, materialAttribute.emissiveColor);

	material->SetName(materialAttribute.name);

	if (materialAttribute.mainTexels != nullptr) {
		material->SetTexture(Variables::mainTexture, CreateTexture2D(materialAttribute.mainTexels));
	}
	else {
		material->SetTexture(Variables::mainTexture, Resources::GetWhiteTexture());
	}

	if (materialAttribute.bumpTexels) {
		material->SetTexture(Variables::bumpTexture, CreateTexture2D(materialAttribute.bumpTexels));
	}

	if (materialAttribute.specularTexels) {
		material->SetTexture(Variables::specularTexture, CreateTexture2D(materialAttribute.specularTexels));
	}

	if (materialAttribute.emissiveTexels) {
		material->SetTexture(Variables::emissiveTexture, CreateTexture2D(materialAttribute.emissiveTexels));
	}

	if (materialAttribute.lightmapTexels) {
		material->SetTexture(Variables::lightmapTexture, CreateTexture2D(materialAttribute.lightmapTexels));
	}
}

void AssetDataLoader::ReadMaterialAttribute(MaterialAttribute& attribute, aiMaterial* material) {
	int aint;
	float afloat;
	aiString astring;
	aiColor3D acolor;

	attribute.shaderName = (scene_->mNumAnimations != 0) ? "lit_animated_texture" : "lit_texture";

	if (material->Get(AI_MATKEY_NAME, astring) == AI_SUCCESS) {
		attribute.name = FileSystem::GetFileName(astring.C_Str());
	}
	
	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), astring) == AI_SUCCESS) {
		attribute.mainTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), astring) == AI_SUCCESS) {
		attribute.bumpTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), astring) == AI_SUCCESS) {
		attribute.specularTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), astring) == AI_SUCCESS) {
		attribute.lightmapTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), astring) == AI_SUCCESS) {
		attribute.emissiveTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_OPACITY, afloat) == AI_SUCCESS) {
		if (Math::Approximately(afloat)) { afloat = 1; }
		attribute.mainColor.a = afloat;
	}

	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, acolor) == AI_SUCCESS) {
		attribute.mainColor = glm::vec4(acolor.r, acolor.g, acolor.b, attribute.mainColor.a);
	}

	if (material->Get(AI_MATKEY_COLOR_SPECULAR, acolor) == AI_SUCCESS) {
		attribute.specularColor = glm::vec3(acolor.r, acolor.g, acolor.b);
	}

	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, acolor) == AI_SUCCESS) {
		attribute.emissiveColor = glm::vec3(acolor.r, acolor.g, acolor.b);
	}

	if (material->Get(AI_MATKEY_SHININESS, afloat) == AI_SUCCESS) {
		attribute.gloss = afloat;
	}

	if (material->Get(AI_MATKEY_TWOSIDED, aint) == AI_SUCCESS) {
		attribute.twoSided = !!aint;
	}
}

bool AssetDataLoader::ReadAnimation(Animation& animation) {
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

void AssetDataLoader::ReadAnimationClip(const aiAnimation* anim, AnimationClip clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapModeLoop);
	ReadAnimationNode(anim, scene_->mRootNode, nullptr);
}

void AssetDataLoader::ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
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

const aiNodeAnim* AssetDataLoader::FindChannel(const aiAnimation* anim, const char* name) {
	for (int i = 0; i < anim->mNumChannels; ++i) {
		if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), name) == 0) {
			return anim->mChannels[i];
		}
	}

	return nullptr;
}

TexelMap* AssetDataLoader::ReadTexels(const std::string& name) {
	TexelMapContainer::iterator pos = texelMapContainer_.find(name);
	if (pos != texelMapContainer_.end()) {
		return pos->second;
	}

	bool status = false;
	TexelMap* answer = MEMORY_CREATE(TexelMap);

	if (String::StartsWith(name, "*")) {
		status = ReadEmbeddedTexels(*answer, String::ToInteger(name.substr(1)));
	}
	else {
		status = ReadExternalTexels(*answer, name);
	}

	if (!status) {
		MEMORY_RELEASE(answer);
		return false;
	}

	texelMapContainer_.insert(std::make_pair(name, answer));
	return answer;
}

bool AssetDataLoader::ReadExternalTexels(TexelMap& texelMap, const std::string& name) {
	return ImageCodec::Decode(texelMap, Resources::GetRootDirectory() + "textures/" + name);
}

bool AssetDataLoader::ReadEmbeddedTexels(TexelMap& texelMap, uint index) {
	if (index >= scene_->mNumTextures) {
		Debug::LogError("embedded texture index out of range");
		return false;
	}

	aiTexture* aitex = scene_->mTextures[index];
	if (aitex->mHeight == 0) {
		if (!ImageCodec::Decode(texelMap, aitex->pcData, aitex->mWidth)) {
			return false;
		}
	}
	else {
		texelMap.textureFormat = TextureFormatRgba;
		texelMap.data.assign((uchar*)aitex->pcData, (uchar*)aitex->pcData + aitex->mWidth * aitex->mHeight * sizeof(aiTexel));
		texelMap.format = ColorStreamFormatArgb;
		texelMap.width = aitex->mWidth;
		texelMap.height = aitex->mHeight;
	}

	return true;
}
