#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <OpenThreads/Thread>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "variables.h"
#include "resources.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "os/filesystem.h"
#include "memory/memory.h"
#include "entityimporter.h"
#include "internal/file/image.h"
#include "internal/base/renderdefines.h"

struct MaterialAsset {
	MaterialAsset();

	void ApplyAsset();
	Texture2D CreateTexture2D(const TexelMap* texelMap);

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

typedef MeshAttribute MeshAsset;

struct EntityAsset {
	MeshAsset meshAsset;
	std::vector<MaterialAsset> materialAssets;
};

class Loader : public OpenThreads::Thread {
public:
	Loader() : callback_(nullptr) {}
	~Loader() { if (isRunning()) { cancel(); } }

public:
	enum {
		Ok,
		Failed,
		Running,
	};

public:
	int Start();
	int GetStatus() const { return status_; }
	void SetCallback(LoaderCallback* value) { callback_ = value; }

public:
	virtual void Run() = 0;

private:
	virtual void run();

protected:
	virtual void Clear() { status_ = Failed; }
	void SetStatus(int value) { status_ = value; }

private:
	int status_;
	LoaderCallback* callback_;
};

int Loader::Start() {
	status_ = Running;
	return start();
}

void Loader::run() {
	Run();
	if (callback_ != nullptr) { (*callback_)(); }
}

class EntityAssetLoader : public Loader {
public:
	~EntityAssetLoader() { Clear(); }

public:
	virtual void Run();

public:
	Mesh GetSurface() { return surface_; }

	EntityAsset& GetEntityAsset() { return asset_; }
	std::vector<Renderer>& GetRenderers() { return renderers_; }

	void SetTarget(const std::string& path, Entity entity);

protected:
	virtual void Clear();

private:
	bool ReadAsset();
	bool Initialize(Assimp::Importer& importer);

	Entity ReadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes);

	void ReadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);
	void ReadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);
	void ReadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);

	void ReserveMemory(MeshAsset& meshAsset);
	bool ReadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes);
	bool ReadAttributeAt(int index, MeshAsset& meshAsset, SubMesh* subMeshes);

	void ReadVertexAttribute(int meshIndex, MeshAsset& meshAsset);
	void ReadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes);

	void ReadMaterials();
	void ReadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material);

	bool ReadAnimation(Animation& animation);
	void ReadAnimationClip(const aiAnimation* anim, AnimationClip clip);
	void ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim * FindChannel(const aiAnimation* anim, const char* name);

	TexelMap* ReadTexels(const std::string& name);

	bool ReadEmbeddedTexels(TexelMap& texelMap, uint index);
	bool ReadExternalTexels(TexelMap& texelMap, const std::string& name);

	glm::vec3& AIVector3ToGLM(glm::vec3& answer, const aiVector3D& vec);
	glm::mat4& AIMaterixToGLM(glm::mat4& answer, const aiMatrix4x4& mat);
	glm::quat& AIQuaternionToGLM(glm::quat& answer, const aiQuaternion& quaternion);
	void DecomposeAIMatrix(glm::vec3& translation, glm::quat& rotation, glm::vec3& scale, const aiMatrix4x4& mat);

private:
	Entity root_;
	Mesh surface_;
	EntityAsset asset_;

	std::string path_;
	Skeleton skeleton_;
	Animation animation_;
	const aiScene* scene_;

	std::vector<Renderer> renderers_;

	typedef std::map<std::string, TexelMap*> TexelMapContainer;
	TexelMapContainer texelMapContainer_;
};

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

void EntityAssetLoader::Run() {
	SetStatus(
		ReadAsset() ? Ok : Failed
	);
}

void EntityAssetLoader::SetTarget(const std::string& path, Entity entity) {
	path_ = path;
	root_ = entity;
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

Entity EntityAssetLoader::ReadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	Entity entity = NewEntity();
	entity->GetTransform()->SetParent(parent->GetTransform());

	ReadNodeTo(entity, node, surface, subMeshes);
	ReadChildren(entity, node, surface, subMeshes);

	return entity;
}

void EntityAssetLoader::ReadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	entity->SetName(node->mName.C_Str());

	glm::vec3 translation, scale;
	glm::quat rotation;
	DecomposeAIMatrix(translation, rotation, scale, node->mTransformation);

	entity->GetTransform()->SetLocalScale(scale);
	entity->GetTransform()->SetLocalRotation(rotation);
	entity->GetTransform()->SetLocalPosition(translation);

	ReadComponents(entity, node, surface, subMeshes);
}

void EntityAssetLoader::ReadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	if (node->mNumMeshes == 0) {
		return;
	}

	Renderer renderer = nullptr;
	if (scene_->mNumAnimations == 0) {
		renderer = NewMeshRenderer();
	}
	else {
		renderer = NewSkinnedMeshRenderer();
		dsp_cast<SkinnedMeshRenderer>(renderer)->SetSkeleton(skeleton_);
	}

	renderers_.push_back(renderer);

	Mesh mesh = NewMesh();
	mesh->ShareStorage(surface);

	for (int i = 0; i < node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		mesh->AddSubMesh(subMeshes[meshIndex]);

		uint materialIndex = scene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(asset_.materialAssets[materialIndex].material);
		}
	}

	entity->SetMesh(mesh);
	entity->SetRenderer(renderer);
}

void EntityAssetLoader::ReadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	for (int i = 0; i < node->mNumChildren; ++i) {
		ReadHierarchy(entity, node->mChildren[i], surface, subMeshes);
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

bool EntityAssetLoader::ReadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes) {
	ReserveMemory(meshAsset);

	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		subMeshes[i] = NewSubMesh();
		TriangleBias bias{ 
			scene_->mMeshes[i]->mNumFaces * 3, meshAsset.indexes.size(), meshAsset.positions.size()
		};
		subMeshes[i]->SetTriangleBias(bias);
		ReadAttributeAt(i, meshAsset, subMeshes);
	}

	return true;
}

bool EntityAssetLoader::ReadAttributeAt(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes) {
	ReadVertexAttribute(meshIndex, meshAsset);
	ReadBoneAttribute(meshIndex, meshAsset, subMeshes);

	return true;
}

void EntityAssetLoader::ReadVertexAttribute(int meshIndex, MeshAsset& meshAsset) {
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

		meshAsset.positions.push_back(glm::vec3(pos->x, pos->y, pos->z));
		meshAsset.normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
		meshAsset.texCoords.push_back(glm::vec2(texCoord->x, texCoord->y));
		meshAsset.tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));
	}

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

void EntityAssetLoader::ReadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes) {
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
					break;
				}
			}
		}
	}
}

void EntityAssetLoader::ReadMaterials() {
	for (int i = 0; i < scene_->mNumMaterials; ++i) {
		ReadMaterialAsset(asset_.materialAssets[i], scene_->mMaterials[i]);
	}
}

void EntityAssetLoader::ReadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material) {
	int aint;
	float afloat;
	aiString astring;
	aiColor3D acolor;

	materialAsset.shaderName = (scene_->mNumAnimations != 0) ? "lit_animated_texture" : "lit_texture";

	if (material->Get(AI_MATKEY_NAME, astring) == AI_SUCCESS) {
		materialAsset.name = FileSystem::GetFileName(astring.C_Str());
	}
	
	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), astring) == AI_SUCCESS) {
		materialAsset.mainTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), astring) == AI_SUCCESS) {
		materialAsset.bumpTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), astring) == AI_SUCCESS) {
		materialAsset.specularTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), astring) == AI_SUCCESS) {
		materialAsset.lightmapTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
	}

	if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), astring) == AI_SUCCESS) {
		materialAsset.emissiveTexels = ReadTexels(FileSystem::GetFileName(astring.C_Str()));
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

bool EntityAssetLoader::ReadAnimation(Animation& animation) {
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

void EntityAssetLoader::ReadAnimationClip(const aiAnimation* anim, AnimationClip clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapModeLoop);
	ReadAnimationNode(anim, scene_->mRootNode, nullptr);
}

void EntityAssetLoader::ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
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

const aiNodeAnim* EntityAssetLoader::FindChannel(const aiAnimation* anim, const char* name) {
	for (int i = 0; i < anim->mNumChannels; ++i) {
		if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), name) == 0) {
			return anim->mChannels[i];
		}
	}

	return nullptr;
}

TexelMap* EntityAssetLoader::ReadTexels(const std::string& name) {
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

bool EntityAssetLoader::ReadExternalTexels(TexelMap& texelMap, const std::string& name) {
	return ImageCodec::Decode(texelMap, Resources::GetRootDirectory() + "textures/" + name);
}

bool EntityAssetLoader::ReadEmbeddedTexels(TexelMap& texelMap, uint index) {
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

bool EntityAssetLoader::ReadAsset() {
	Assimp::Importer importer;
	if (!Initialize(importer)) {
		return false;
	}

	SubMesh* subMeshes = nullptr;
	asset_.meshAsset.topology = MeshTopologyTriangles;

	if (scene_->mNumMaterials > 0) {
		asset_.materialAssets.resize(scene_->mNumMaterials);
		ReadMaterials();
	}

	if (scene_->mNumMeshes > 0) {
		subMeshes = MEMORY_CREATE_ARRAY(SubMesh, scene_->mNumMeshes);
		if (!ReadAttribute(asset_.meshAsset, subMeshes)) {
			Debug::LogError("failed to load meshes for %s.", path_.c_str());
		}
	}

	surface_ = NewMesh();
	surface_->CreateStorage();

	ReadNodeTo(root_, scene_->mRootNode, surface_, subMeshes);
	ReadChildren(root_, scene_->mRootNode, surface_, subMeshes);

	MEMORY_RELEASE_ARRAY(subMeshes);

	Animation animation;
	if (ReadAnimation(animation)) {
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

glm::vec3& EntityAssetLoader::AIVector3ToGLM(glm::vec3& answer, const aiVector3D& vec) {
	answer = glm::vec3(vec.x, vec.y, vec.z);
	return answer;
}

EntityImporter::EntityImporter() 
	: loader_(MEMORY_CREATE(EntityAssetLoader))
	, status_(Loader::Failed) {
	loader_->SetCallback(this);
}

EntityImporter::~EntityImporter() {
	MEMORY_RELEASE(loader_);
}

void EntityImporter::operator()() {
	status_ = Loader::Ok;
}

Entity EntityImporter::Import(const std::string& path) {
	Entity entity = NewEntity();
	ImportTo(entity, path);
	return entity;
}

bool EntityImporter::ImportTo(Entity entity, const std::string& path) {
	if (!entity) {
		Debug::LogError("invalid entity");
		return false;
	}

	if (status_ != Loader::Failed) {
		Debug::LogError("asset importer is running");
		return false;
	}

	loader_->SetTarget(path, entity);

	int err = loader_->Start();
	if (err != 0) {
		Debug::LogError("failed to start thread: %s", StrError(err).c_str());
		return false;
	}

	return true;
}

void EntityImporter::Update() {
	if (status_ != Loader::Ok) {
		return;
	}

	status_ = Loader::Failed;
	EntityAsset asset = loader_->GetEntityAsset();
	for (uint i = 0; i < asset.materialAssets.size(); ++i) {
		asset.materialAssets[i].ApplyAsset();
	}

	loader_->GetSurface()->SetAttribute(asset.meshAsset);

	typedef std::vector<Renderer> Renderers;
	Renderers renderers = loader_->GetRenderers();
	for (Renderers::iterator ite = renderers.begin(); ite != renderers.end(); ++ite) {
		(*ite)->SetReady(true);
	}
}

std::string EntityImporter::StrError(int err) {
	if (err == -1) {
		return "errno = " + std::to_string(errno);
	}

	return "error = " + std::to_string(err);
}

