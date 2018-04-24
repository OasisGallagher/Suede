#include <glm/gtx/matrix_decompose.hpp>

#include "bounds.h"
#include "variables.h"
#include "resources.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "memory/memory.h"
#include "os/filesystem.h"
#include "entityassetloader.h"
#include "internal/base/renderdefines.h"

#define LockPathInScope()	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(pathMutex_)

int Loader::Start() {
	status_ = Running;
	return start();
}

void Loader::run() {
	for (; status_ != Done;) {
		if (!IsReady()) {
			OpenThreads::Thread::YieldCurrentThread();
			continue;
		}

		Run();
		if (listener_ != nullptr) { listener_->OnLoadFinished(); }
	}
}

MaterialAsset::MaterialAsset()
	: twoSided(false), gloss(20), mainColor(1), name(UNNAMED_MATERIAL) {
	material = NewMaterial();
	mainTexels = bumpTexels = specularTexels = emissiveTexels = lightmapTexels = nullptr;
}

void MaterialAsset::ApplyAsset() {
	Shader shader = Resources::FindShader("buildin/shaders/" + shaderName);
	material->SetShader(shader);

	material->SetFloat(Variables::gloss, gloss);

	material->SetColor4(Variables::mainColor, mainColor);
	material->SetColor3(Variables::specularColor, specularColor);
	material->SetColor3(Variables::emissiveColor, emissiveColor);

	material->SetName(name);

	if (mainTexels != nullptr) {
		material->SetTexture(Variables::mainTexture, CreateTexture2D(mainTexels));
	}
	else {
		material->SetTexture(Variables::mainTexture, Resources::GetWhiteTexture());
	}

	if (bumpTexels != nullptr) {
		material->SetTexture(Variables::bumpTexture, CreateTexture2D(bumpTexels));
	}

	if (specularTexels != nullptr) {
		material->SetTexture(Variables::specularTexture, CreateTexture2D(specularTexels));
	}

	if (emissiveTexels != nullptr) {
		material->SetTexture(Variables::emissiveTexture, CreateTexture2D(emissiveTexels));
	}

	if (lightmapTexels != nullptr) {
		material->SetTexture(Variables::lightmapTexture, CreateTexture2D(lightmapTexels));
	}
}

Texture2D MaterialAsset::CreateTexture2D(const TexelMap* texelMap) {
	Texture2D texture = NewTexture2D();
	if (!texture->Load(texelMap->textureFormat, &texelMap->data[0], texelMap->format, texelMap->width, texelMap->height, false)) {
		return nullptr;
	}

	return texture;
}

bool EntityAssetLoader::Initialize(Assimp::Importer &importer) {
	uint flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
		| aiProcess_ImproveCacheLocality | aiProcess_FindInstances | aiProcess_GenSmoothNormals
		| aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_OptimizeMeshes/* | aiProcess_OptimizeGraph*/
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

void EntityAssetLoader::Run() {
	SetStatus(
		LoadAsset() ? Ok : Failed
	);

	root_.reset();
	path_.clear();
}

bool EntityAssetLoader::IsReady() const {
	return !path_.empty();
}

bool EntityAssetLoader::Load(const std::string& path, Entity entity) {
	if (!path_.empty()) {
		Debug::LogError("loader is running.");
		return false;
	}

	path_ = path;
	root_ = entity;
	root_->SetStatus(EntityStatusLoading);

	return true;
}

void EntityAssetLoader::Clear() {
	Loader::Clear();

	path_.clear();

	asset_.meshAsset = MeshAsset();
	asset_.materialAssets.clear();

	scene_ = nullptr;
	surface_.reset();
	skeleton_.reset();

	for (TexelMapContainer::iterator ite = texelMapContainer_.begin(); ite != texelMapContainer_.end(); ++ite) {
		MEMORY_RELEASE(ite->second);
	}

	texelMapContainer_.clear();

	animation_.reset();
}

Entity EntityAssetLoader::LoadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses) {
	Entity entity = NewEntity();
	entity->GetTransform()->SetParent(parent->GetTransform());

	LoadNodeTo(entity, node, surface, subMeshes, boundses);
	LoadChildren(entity, node, surface, subMeshes, boundses);

	return entity;
}

void EntityAssetLoader::LoadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses) {
	entity->SetName(node->mName.C_Str());

	if (entity != root_) {
		glm::quat rotation;
		glm::vec3 translation, scale;
		DecomposeAIMatrix(translation, rotation, scale, node->mTransformation);

		entity->GetTransform()->SetLocalScale(scale);
		entity->GetTransform()->SetLocalRotation(rotation);
		entity->GetTransform()->SetLocalPosition(translation);
	}

	LoadComponents(entity, node, surface, subMeshes, boundses);
}

void EntityAssetLoader::LoadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses) {
	if (node->mNumMeshes == 0) {
		return;
	}

	Renderer renderer = nullptr;
	if (scene_->mNumAnimations == 0) {
		renderer = NewMeshRenderer();
	}
	else {
		renderer = NewSkinnedMeshRenderer();
		suede_dynamic_cast<SkinnedMeshRenderer>(renderer)->SetSkeleton(skeleton_);
	}

	Mesh mesh = NewMesh();
	mesh->ShareStorage(surface);
	
	Bounds bounds(boundses[node->mMeshes[0]]);
	for (int i = 0; i < node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		mesh->AddSubMesh(subMeshes[meshIndex]);

		if (i > 0) {
			bounds.Encapsulate(boundses[meshIndex]);
		}

		uint materialIndex = scene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(asset_.materialAssets[materialIndex].material);
		}
	}

	mesh->SetBounds(bounds);

	entity->SetMesh(mesh);
	entity->SetRenderer(renderer);
}

void EntityAssetLoader::LoadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses) {
	for (int i = 0; i < node->mNumChildren; ++i) {
		LoadHierarchy(entity, node->mChildren[i], surface, subMeshes, boundses);
	}
}

void EntityAssetLoader::ReserveMemory(MeshAsset& meshAsset) {
	int indexCount = 0, vertexCount = 0;
	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		indexCount += scene_->mMeshes[i]->mNumFaces * 3;
		vertexCount += scene_->mMeshes[i]->mNumVertices;
	}

	meshAsset.positions.reserve(vertexCount);
	meshAsset.normals.reserve(vertexCount);
	meshAsset.texCoords.reserve(vertexCount);
	meshAsset.tangents.reserve(vertexCount);
	meshAsset.indexes.reserve(indexCount);
	meshAsset.blendAttrs.resize(vertexCount);
}

bool EntityAssetLoader::LoadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes, Bounds* boundses) {
	ReserveMemory(meshAsset);

	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		subMeshes[i] = NewSubMesh();
		TriangleBias bias {
			scene_->mMeshes[i]->mNumFaces * 3, meshAsset.indexes.size(), meshAsset.positions.size()
		};
		subMeshes[i]->SetTriangleBias(bias);
		LoadAttributeAt(i, meshAsset, subMeshes, boundses);
	}

	return true;
}

bool EntityAssetLoader::LoadAttributeAt(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes, Bounds* boundses) {
	LoadVertexAttribute(meshIndex, meshAsset, boundses);
	LoadBoneAttribute(meshIndex, meshAsset, subMeshes);

	return true;
}

void EntityAssetLoader::LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset, Bounds* boundses) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];

	// TODO: multiple texture coords?
	for (int i = 1; i < AI_MAX_NUMBER_OF_COLOR_SETS; ++i) {
		if (aimesh->HasTextureCoords(i)) {
			Debug::LogWarning("multiple texture coordinates");
		}
	}

	//bool logged = false;
	const aiVector3D zero(0);
	
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < aimesh->mNumVertices; ++i) {
		glm::vec3 pos = AIVector3ToGLM(aimesh->mVertices[i]);
		glm::vec3 normal = AIVector3ToGLM(aimesh->mNormals[i]);

		// TODO:
		//if (aimesh->GetNumUVChannels() != 1 && !logged) {
		//	logged = true;
		//	Debug::LogWarning("this mesh contains %d uv channel(s).", aimesh->GetNumUVChannels());
		//}

		const aiVector3D* texCoord = aimesh->HasTextureCoords(0) ? &(aimesh->mTextureCoords[0][i]) : &zero;
		const aiVector3D* tangent = (aimesh->mTangents != nullptr) ? &aimesh->mTangents[i] : &zero;

		meshAsset.positions.push_back(pos);
		meshAsset.normals.push_back(normal);
		meshAsset.texCoords.push_back(glm::vec2(texCoord->x, texCoord->y));
		meshAsset.tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));

		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}

	boundses[meshIndex].SetMinMax(min, max);

	for (uint i = 0; i < aimesh->mNumFaces; ++i) {
		const aiFace& face = aimesh->mFaces[i];
		if (face.mNumIndices != 3) {
			Debug::LogError("invalid index count");
			continue;
		}

		meshAsset.indexes.push_back(face.mIndices[0]);
		meshAsset.indexes.push_back(face.mIndices[1]);
		meshAsset.indexes.push_back(face.mIndices[2]);
	}
}

void EntityAssetLoader::LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes) {
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
				if (Math::Approximately(meshAsset.blendAttrs[vertexID].weights[k])) {
					meshAsset.blendAttrs[vertexID].indexes[k] = index;
					meshAsset.blendAttrs[vertexID].weights[k] = weight;

					SkeletonBone* bone = skeleton_->GetBone(index);
					bone->bounds.Encapsulate(glm::vec3(bone->localToBoneMatrix * glm::vec4(meshAsset.positions[vertexID], 1)));
					break;
				}
			}
		}
	}
}

void EntityAssetLoader::LoadMaterialAssets() {
	for (int i = 0; i < scene_->mNumMaterials; ++i) {
		LoadMaterialAsset(asset_.materialAssets[i], scene_->mMaterials[i]);
	}
}

void EntityAssetLoader::LoadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material) {
	int aint;
	float afloat;
	aiString astring;
	aiColor3D acolor;

	materialAsset.shaderName = (scene_->mNumAnimations != 0) ? "lit_animated_texture" : "lit_texture";

	if (material->Get(AI_MATKEY_NAME, astring) == AI_SUCCESS) {
		materialAsset.name = FileSystem::GetFileName(astring.C_Str());
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), astring) == AI_SUCCESS) {
		materialAsset.mainTexels = LoadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), astring) == AI_SUCCESS) {
		materialAsset.bumpTexels = LoadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), astring) == AI_SUCCESS) {
		materialAsset.specularTexels = LoadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), astring) == AI_SUCCESS) {
		materialAsset.lightmapTexels = LoadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), astring) == AI_SUCCESS) {
		materialAsset.emissiveTexels = LoadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_OPACITY, afloat) == AI_SUCCESS) {
		if (Math::Approximately(afloat)) { afloat = 1; }
		materialAsset.mainColor.a = afloat;
	}

	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, acolor) == AI_SUCCESS) {
		materialAsset.mainColor = glm::vec4(acolor.r, acolor.g, acolor.b, materialAsset.mainColor.a);
	}

	if (material->Get(AI_MATKEY_COLOR_SPECULAR, acolor) == AI_SUCCESS) {
		materialAsset.specularColor = glm::vec3(acolor.r, acolor.g, acolor.b);
	}

	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, acolor) == AI_SUCCESS) {
		materialAsset.emissiveColor = glm::vec3(acolor.r, acolor.g, acolor.b);
	}

	if (material->Get(AI_MATKEY_SHININESS, afloat) == AI_SUCCESS) {
		materialAsset.gloss = afloat;
	}

	if (material->Get(AI_MATKEY_TWOSIDED, aint) == AI_SUCCESS) {
		materialAsset.twoSided = !!aint;
	}
}

bool EntityAssetLoader::LoadAnimation(Animation& animation) {
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

		LoadAnimationClip(anim, clip);
		animation->AddClip(name, clip);
	}

	animation->SetSkeleton(skeleton_);
	animation->Play(defaultClipName);

	return true;
}

void EntityAssetLoader::LoadAnimationClip(const aiAnimation* anim, AnimationClip clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapModeLoop);
	LoadAnimationNode(anim, scene_->mRootNode, nullptr);
}

void EntityAssetLoader::LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
	const aiNodeAnim* channel = FindChannel(anim, paiNode->mName.C_Str());

	AnimationCurve curve;
	AnimationKeys keys = NewAnimationKeys();
	if (channel != nullptr) {
		for (int i = 0; i < channel->mNumPositionKeys; ++i) {
			const aiVectorKey& key = channel->mPositionKeys[i];
			keys->AddVector3(FrameKeyPosition, (float)key.mTime, AIVector3ToGLM(key.mValue));
		}

		for (int i = 0; i < channel->mNumRotationKeys; ++i) {
			const aiQuatKey& key = channel->mRotationKeys[i];
			glm::quat rotation;
			keys->AddQuaternion(FrameKeyRotation, (float)key.mTime, AIQuaternionToGLM(rotation, key.mValue));
		}

		for (int i = 0; i < channel->mNumScalingKeys; ++i) {
			const aiVectorKey& key = channel->mScalingKeys[i];
			keys->AddVector3(FrameKeyScale, (float)key.mTime, AIVector3ToGLM(key.mValue));
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
		LoadAnimationNode(anim, paiNode->mChildren[i], child);
	}
}

const aiNodeAnim* EntityAssetLoader::FindChannel(const aiAnimation* anim, const char* name) {
	for (int i = 0; i < anim->mNumChannels; ++i) {
		if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), name) == 0) {
			return anim->mChannels[i];
		}
	}

	return nullptr;
}

TexelMap* EntityAssetLoader::LoadTexels(const std::string& name) {
	TexelMapContainer::iterator pos = texelMapContainer_.find(name);
	if (pos != texelMapContainer_.end()) {
		return pos->second;
	}

	bool status = false;
	TexelMap* answer = MEMORY_CREATE(TexelMap);

	if (String::StartsWith(name, "*")) {
		status = LoadEmbeddedTexels(*answer, String::ToInteger(name.substr(1)));
	}
	else {
		status = LoadExternalTexels(*answer, name);
	}

	if (!status) {
		MEMORY_RELEASE(answer);
		return false;
	}

	texelMapContainer_.insert(std::make_pair(name, answer));
	return answer;
}

bool EntityAssetLoader::LoadExternalTexels(TexelMap& texelMap, const std::string& name) {
	return ImageCodec::Decode(texelMap, Resources::GetRootDirectory() + "textures/" + name);
}

bool EntityAssetLoader::LoadEmbeddedTexels(TexelMap& texelMap, uint index) {
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

bool EntityAssetLoader::LoadAsset() {
	Assimp::Importer importer;
	if (!Initialize(importer)) {
		return false;
	}

	SubMesh* subMeshes = nullptr;
	Bounds* boundses = nullptr;
	asset_.meshAsset.topology = MeshTopologyTriangles;

	if (scene_->mNumMaterials > 0) {
		asset_.materialAssets.resize(scene_->mNumMaterials);
		LoadMaterialAssets();
	}

	if (scene_->mNumMeshes > 0) {
		subMeshes = MEMORY_CREATE_ARRAY(SubMesh, scene_->mNumMeshes);
		boundses = MEMORY_CREATE_ARRAY(Bounds, scene_->mNumMeshes);
		if (!LoadAttribute(asset_.meshAsset, subMeshes, boundses)) {
			Debug::LogError("failed to load meshes for %s.", path_.c_str());
		}
	}

	surface_ = NewMesh();
	surface_->CreateStorage();

	LoadNodeTo(root_, scene_->mRootNode, surface_, subMeshes, boundses);
	LoadChildren(root_, scene_->mRootNode, surface_, subMeshes, boundses);

	MEMORY_RELEASE_ARRAY(subMeshes);
	MEMORY_RELEASE_ARRAY(boundses);

	Animation animation;
	if (LoadAnimation(animation)) {
		root_->SetAnimation(animation);
	}

	return true;
}

glm::mat4& EntityAssetLoader::AIMaterixToGLM(glm::mat4& answer, const aiMatrix4x4& mat) {
	answer = glm::mat4(
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
	);

	return answer;
}

glm::quat& EntityAssetLoader::AIQuaternionToGLM(glm::quat& answer, const aiQuaternion& quaternion) {
	answer = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
	return answer;
}

void EntityAssetLoader::DecomposeAIMatrix(glm::vec3& translation, glm::quat& rotation, glm::vec3& scale, const aiMatrix4x4& mat) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::mat4 transformation;
	AIMaterixToGLM(transformation, mat);
	glm::decompose(transformation, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);
}

glm::vec3 EntityAssetLoader::AIVector3ToGLM(const aiVector3D& vec) {
	return glm::vec3(vec.x, vec.y, vec.z);
}
