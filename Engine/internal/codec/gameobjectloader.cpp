#include "gameobjectloader.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/gtx/matrix_decompose.hpp>

#include "world.h"
#include "resources.h"
#include "memory/memory.h"
#include "os/filesystem.h"
#include "builtinproperties.h"

inline glm::vec3 AIConvert(const aiVector3D& vec) {
	return glm::vec3(vec.x, vec.y, vec.z);
}

inline glm::mat4& AIConvert(glm::mat4& answer, const aiMatrix4x4& mat) {
	answer = glm::mat4(
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
	);

	return answer;
}

inline glm::quat& AIConvert(glm::quat& answer, const aiQuaternion& quaternion) {
	answer = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
	return answer;
}

inline void AIConvert(glm::vec3& translation, glm::quat& rotation, glm::vec3& scale, const aiMatrix4x4& mat) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::mat4 transformation;
	AIConvert(transformation, mat);
	glm::decompose(transformation, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);
}

#define UNNAMED_MATERIAL	"New Material"

MaterialAsset::MaterialAsset()
	: twoSided(false), gloss(50), mainColor(Color::white), name(UNNAMED_MATERIAL) {
	material = NewMaterial();
	mainTexels = bumpTexels = specularTexels = emissiveTexels = lightmapTexels = nullptr;
}

void MaterialAsset::ApplyAsset() {
	Shader shader = Resources::FindShader("builtin/" + shaderName);
	material->SetShader(shader);

	material->SetFloat(BuiltinProperties::Gloss, gloss);

	material->SetColor(BuiltinProperties::MainColor, mainColor);
	material->SetColor(BuiltinProperties::SpecularColor, specularColor);
	material->SetColor(BuiltinProperties::EmissiveColor, emissiveColor);

	material->SetName(name);

	if (mainTexels != nullptr) {
		material->SetTexture(BuiltinProperties::MainTexture, CreateTexture2D(mainTexels));
	}
	else {
		material->SetTexture(BuiltinProperties::MainTexture, Resources::GetWhiteTexture());
	}

	if (bumpTexels != nullptr) {
		material->SetTexture(BuiltinProperties::BumpTexture, CreateTexture2D(bumpTexels));
	}

	if (specularTexels != nullptr) {
		material->SetTexture(BuiltinProperties::SpecularTexture, CreateTexture2D(specularTexels));
	}

	if (emissiveTexels != nullptr) {
		material->SetTexture(BuiltinProperties::EmissiveTexture, CreateTexture2D(emissiveTexels));
	}

	if (lightmapTexels != nullptr) {
		material->SetTexture(BuiltinProperties::LightmapTexture, CreateTexture2D(lightmapTexels));
	}
}

Texture2D MaterialAsset::CreateTexture2D(const TexelMap* texelMap) {
	Texture2D texture = NewTexture2D();
	if (!texture->Create(texelMap->textureFormat, &texelMap->data[0], texelMap->colorStreamFormat, texelMap->width, texelMap->height, 4, false)) {
		return nullptr;
	}

	return texture;
}

GameObjectLoader::GameObjectLoader(const std::string& path, GameObject root, WorkerEventListener* receiver)
	: Worker(receiver), path_(path), root_(root) {
}

GameObjectLoader::~GameObjectLoader() {
	for (TexelMapContainer::iterator ite = texelMapContainer_.begin(); ite != texelMapContainer_.end(); ++ite) {
		MEMORY_DELETE(ite->second);
	}
}

void GameObjectLoader::Run() {
	if (!LoadAsset()) {
	}
}

bool GameObjectLoader::Initialize(Assimp::Importer& importer) {
	uint flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
		| aiProcess_ImproveCacheLocality | aiProcess_FindInstances | aiProcess_GenSmoothNormals
		| aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_OptimizeMeshes/* | aiProcess_OptimizeGraph*/
		| aiProcess_RemoveRedundantMaterials;

	if (FileSystem::GetExtension(path_) == ".fbx") {
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	}

	std::string fpath = Resources::GetModelDirectory() + path_;

	const aiScene* scene = importer.ReadFile(fpath.c_str(), flags);
	if (scene == nullptr) {
		Debug::LogError("failed to read file %s: %s", path_.c_str(), importer.GetErrorString());
		return false;
	}

	scene_ = scene;

	return true;
}

GameObject GameObjectLoader::LoadHierarchy(GameObject parent, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	GameObject go = NewGameObject();
	go->GetTransform()->SetParent(parent->GetTransform());

	LoadNodeTo(go, node, surface, subMeshes);
	LoadChildren(go, node, surface, subMeshes);

	return go;
}

void GameObjectLoader::LoadNodeTo(GameObject go, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	go->SetName(node->mName.C_Str());

	if (go != root_) {
		glm::quat rotation;
		glm::vec3 translation, scale;
		AIConvert(translation, rotation, scale, node->mTransformation);

		go->GetTransform()->SetLocalScale(scale);
		go->GetTransform()->SetLocalRotation(rotation);
		go->GetTransform()->SetLocalPosition(translation);
	}

	LoadComponents(go, node, surface, subMeshes);
}

void GameObjectLoader::LoadComponents(GameObject go, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	if (node->mNumMeshes == 0) {
		return;
	}

	Renderer renderer = nullptr;
	if (!HasAnimation()) {
		renderer = std::make_shared<IMeshRenderer>();
	}
	else {
		renderer = std::make_shared<ISkinnedMeshRenderer>();
		suede_dynamic_cast<SkinnedMeshRenderer>(renderer)->SetSkeleton(skeleton_);
	}

	asset_.components.push_back(std::make_pair(go, renderer));

	Mesh mesh = NewMesh();
	mesh->ShareStorage(surface);

	MeshFilter meshFilter = std::make_shared<IMeshFilter>();
	meshFilter->SetMesh(mesh);
	asset_.components.push_back(std::make_pair(go, meshFilter));

	for (int i = 0; i < node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		mesh->AddSubMesh(subMeshes[meshIndex]);

		uint materialIndex = scene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(asset_.materialAssets[materialIndex].material);
		}
	}
}

void GameObjectLoader::LoadChildren(GameObject go, aiNode* node, Mesh& surface, SubMesh* subMeshes) {
	for (int i = 0; i < node->mNumChildren; ++i) {
		LoadHierarchy(go, node->mChildren[i], surface, subMeshes);
	}
}

void GameObjectLoader::ReserveMemory(MeshAsset& meshAsset) {
	uint indexCount = 0, vertexCount = 0;
	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		indexCount += scene_->mMeshes[i]->mNumFaces * 3;
		vertexCount += scene_->mMeshes[i]->mNumVertices;
	}

	meshAsset.positions.reserve(vertexCount);
	meshAsset.normals.reserve(vertexCount);

	for (int i = 0; i < MeshAttribute::TexCoordsCount; ++i) {
		meshAsset.texCoords[i].reserve(vertexCount);
	}

	meshAsset.tangents.reserve(vertexCount);
	meshAsset.indexes.reserve(indexCount);
	meshAsset.blendAttrs.resize(vertexCount);
}

bool GameObjectLoader::LoadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes) {
	ReserveMemory(meshAsset);

	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		subMeshes[i] = NewSubMesh();
		TriangleBias bias{
			scene_->mMeshes[i]->mNumFaces * 3, meshAsset.indexes.size(), meshAsset.positions.size()
		};
		subMeshes[i]->SetTriangleBias(bias);
		LoadAttributeAt(i, meshAsset, subMeshes);
	}

	return true;
}

bool GameObjectLoader::LoadAttributeAt(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes) {
	LoadVertexAttribute(meshIndex, meshAsset);
	LoadBoneAttribute(meshIndex, meshAsset, subMeshes);

	return true;
}

void GameObjectLoader::LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];

	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < aimesh->mNumVertices; ++i) {
		glm::vec3 pos = AIConvert(aimesh->mVertices[i]);
		glm::vec3 normal = AIConvert(aimesh->mNormals[i]);

		for (int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
			if (!aimesh->HasTextureCoords(j)) {
				continue;
			}

			if (j > MeshAttribute::TexCoordsCount) {
				Debug::LogWarning("only %d texture coordinates are supported.", MeshAttribute::TexCoordsCount);
				break;
			}

			const aiVector3D& v3 = aimesh->mTextureCoords[j][i];
			meshAsset.texCoords[j].push_back(glm::vec2(v3.x, v3.y));
		}

		if (aimesh->mTangents != nullptr) {
			const aiVector3D& v3 = aimesh->mTangents[i];
			meshAsset.tangents.push_back(glm::vec3(v3.x, v3.y, v3.z));
		}

		meshAsset.positions.push_back(pos);
		meshAsset.normals.push_back(normal);

		min = glm::min(min, pos);
		max = glm::max(max, pos);
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

void GameObjectLoader::LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];
	for (int i = 0; i < aimesh->mNumBones; ++i) {
		if (!skeleton_) { skeleton_ = NewSkeleton(); }
		std::string name(aimesh->mBones[i]->mName.data);

		int index = skeleton_->GetBoneIndex(name);
		if (index < 0) {
			SkeletonBone bone{ name };
			AIConvert(bone.localToBoneMatrix, aimesh->mBones[i]->mOffsetMatrix);
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

void GameObjectLoader::LoadMaterialAssets() {
	for (int i = 0; i < scene_->mNumMaterials; ++i) {
		LoadMaterialAsset(asset_.materialAssets[i], scene_->mMaterials[i]);
	}
}

void GameObjectLoader::LoadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material) {
	int aint;
	float afloat;
	aiString astring;
	aiColor3D acolor;

	materialAsset.shaderName = HasAnimation() ? "lit_animated_texture" : "lit_texture";

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
		materialAsset.mainColor = Color(acolor.r, acolor.g, acolor.b, materialAsset.mainColor.a);
	}

	if (material->Get(AI_MATKEY_COLOR_SPECULAR, acolor) == AI_SUCCESS) {
		materialAsset.specularColor = Color(acolor.r, acolor.g, acolor.b, 1);
	}

	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, acolor) == AI_SUCCESS) {
		materialAsset.emissiveColor = Color(acolor.r, acolor.g, acolor.b, 1);
	}

	if (material->Get(AI_MATKEY_SHININESS, afloat) == AI_SUCCESS) {
		materialAsset.gloss = afloat;
	}

	if (material->Get(AI_MATKEY_TWOSIDED, aint) == AI_SUCCESS) {
		materialAsset.twoSided = !!aint;
	}
}

void GameObjectLoader::LoadAnimation(Animation animation) {
	glm::mat4 rootTransform;
	animation->SetRootTransform(AIConvert(rootTransform, scene_->mRootNode->mTransformation.Inverse()));

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
}

bool GameObjectLoader::HasAnimation() {
	return skeleton_ && scene_->mNumAnimations != 0;
}

void GameObjectLoader::LoadAnimationClip(const aiAnimation* anim, AnimationClip clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapMode::Loop);
	LoadAnimationNode(anim, scene_->mRootNode, nullptr);
}

void GameObjectLoader::LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
	const aiNodeAnim* channel = FindChannel(anim, paiNode->mName.C_Str());

	AnimationCurve curve;
	AnimationKeys keys = NewAnimationKeys();
	if (channel != nullptr) {
		for (int i = 0; i < channel->mNumPositionKeys; ++i) {
			const aiVectorKey& key = channel->mPositionKeys[i];
			keys->AddVector3((float)key.mTime, FrameKeyPosition, AIConvert(key.mValue));
		}

		for (int i = 0; i < channel->mNumRotationKeys; ++i) {
			const aiQuatKey& key = channel->mRotationKeys[i];
			glm::quat rotation;
			keys->AddQuaternion((float)key.mTime, FrameKeyRotation, AIConvert(rotation, key.mValue));
		}

		for (int i = 0; i < channel->mNumScalingKeys; ++i) {
			const aiVectorKey& key = channel->mScalingKeys[i];
			keys->AddVector3((float)key.mTime, FrameKeyScale, AIConvert(key.mValue));
		}

		std::vector<AnimationFrame> keyframes;
		keys->ToKeyframes(keyframes);

		curve = NewAnimationCurve();
		curve->SetKeyframes(keyframes);
	}

	glm::mat4 matrix;
	SkeletonNode* child = skeleton_->CreateNode(paiNode->mName.C_Str(), AIConvert(matrix, paiNode->mTransformation), curve);
	skeleton_->AddNode(pskNode, child);

	for (int i = 0; i < paiNode->mNumChildren; ++i) {
		LoadAnimationNode(anim, paiNode->mChildren[i], child);
	}
}

const aiNodeAnim* GameObjectLoader::FindChannel(const aiAnimation* anim, const char* name) {
	for (int i = 0; i < anim->mNumChannels; ++i) {
		if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), name) == 0) {
			return anim->mChannels[i];
		}
	}

	return nullptr;
}

TexelMap* GameObjectLoader::LoadTexels(const std::string& name) {
	TexelMapContainer::iterator pos = texelMapContainer_.find(name);
	if (pos != texelMapContainer_.end()) {
		return pos->second;
	}

	bool status = false;
	TexelMap* answer = MEMORY_NEW(TexelMap);

	if (String::StartsWith(name, "*")) {
		status = LoadEmbeddedTexels(*answer, String::ToInteger(name.substr(1)));
	}
	else {
		status = LoadExternalTexels(*answer, FileSystem::GetFileNameWithoutExtension(path_) + "/" + name);
	}

	if (!status) {
		MEMORY_DELETE(answer);
		return false;
	}

	texelMapContainer_.insert(std::make_pair(name, answer));
	return answer;
}

bool GameObjectLoader::LoadExternalTexels(TexelMap& texelMap, const std::string& name) {
	return ImageCodec::Decode(texelMap, Resources::GetTextureDirectory() + name);
}

bool GameObjectLoader::LoadEmbeddedTexels(TexelMap& texelMap, uint index) {
	SUEDE_VERIFY_INDEX(index, scene_->mNumTextures, false);

	aiTexture* aitex = scene_->mTextures[index];
	if (aitex->mHeight == 0) {
		if (!ImageCodec::Decode(texelMap, aitex->pcData, aitex->mWidth)) {
			return false;
		}
	}
	else {
		texelMap.textureFormat = TextureFormat::Rgba;
		texelMap.data.assign((uchar*)aitex->pcData, (uchar*)aitex->pcData + aitex->mWidth * aitex->mHeight * sizeof(aiTexel));
		texelMap.colorStreamFormat = ColorStreamFormat::Argb;
		texelMap.width = aitex->mWidth;
		texelMap.height = aitex->mHeight;
	}

	return true;
}

bool GameObjectLoader::LoadAsset() {
	Assimp::Importer importer;
	if (!Initialize(importer)) {
		return false;
	}

	SubMesh* subMeshes = nullptr;
	asset_.meshAsset.topology = MeshTopology::Triangles;

	if (scene_->mNumMeshes > 0) {
		subMeshes = MEMORY_NEW_ARRAY(SubMesh, scene_->mNumMeshes);
		if (!LoadAttribute(asset_.meshAsset, subMeshes)) {
			Debug::LogError("failed to load meshes for %s.", path_.c_str());
		}
	}

	if (scene_->mNumMaterials > 0) {
		asset_.materialAssets.resize(scene_->mNumMaterials);
		LoadMaterialAssets();
	}

	surface_ = NewMesh();
	surface_->CreateStorage();

	LoadNodeTo(root_, scene_->mRootNode, surface_, subMeshes);
	LoadChildren(root_, scene_->mRootNode, surface_, subMeshes);

	MEMORY_DELETE_ARRAY(subMeshes);

	if (HasAnimation()) {
		Animation animation = std::make_shared<IAnimation>();
		asset_.components.push_back(std::make_pair(root_, animation));
		LoadAnimation(animation);
	}

	return true;
}

GameObject GameObjectLoaderThreadPool::Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback) {
	GameObject root = NewGameObject();
	ImportTo(root, path, callback);
	return root;
}

bool GameObjectLoaderThreadPool::ImportTo(GameObject go, const std::string& path, Lua::Func<void, GameObject, const std::string&> callback) {
	return Execute(MEMORY_NEW(GameObjectLoaderWithCallback, path, go, this, callback));
}

void GameObjectLoaderThreadPool::OnSchedule(ZThread::Task& schedule) {
	GameObjectLoaderWithCallback* loader = (GameObjectLoaderWithCallback*)schedule.get();

	GameObjectAsset asset = loader->GetGameObjectAsset();
	for (uint i = 0; i < asset.materialAssets.size(); ++i) {
		asset.materialAssets[i].ApplyAsset();
	}

	for (uint i = 0; i < asset.components.size(); ++i) {
		asset.components[i].first->AddComponent(asset.components[i].second);
	}

	loader->GetSurface()->SetAttribute(asset.meshAsset);

	GameObject root = loader->GetGameObject();
	root->GetTransform()->SetParent(World::GetRootTransform());

	if (loader->GetParameter()) {
		(*loader->GetParameter())(root, loader->GetPath());
	}

	if (listener_ != nullptr) {
		listener_->OnGameObjectImported(root, loader->GetPath());
	}
}
