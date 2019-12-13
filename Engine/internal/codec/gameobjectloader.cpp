#include "gameobjectloader.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "world.h"
#include "renderer.h"
#include "resources.h"
#include "memory/refptr.h"
#include "os/filesystem.h"
#include "builtinproperties.h"

inline Vector3 AIConvert(const aiVector3D& vec) {
	return Vector3(vec.x, vec.y, vec.z);
}

inline Matrix4& AIConvert(Matrix4& answer, const aiMatrix4x4& mat) {
	answer = Matrix4(
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
	);

	return answer;
}

inline Quaternion& AIConvert(Quaternion& answer, const aiQuaternion& quaternion) {
	answer = Quaternion(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
	return answer;
}

inline void AIConvert(Vector3& translation, Quaternion& rotation, Vector3& scale, const aiMatrix4x4& mat) {
	Vector3 skew;
	Vector4 perspective;
	Matrix4 transformation;
	AIConvert(transformation, mat);
	Matrix4::Decompose(transformation, scale, rotation, translation, skew, perspective);
	rotation = rotation.GetConjugated();
}

#define UNNAMED_MATERIAL	"New Material"

MaterialAsset::MaterialAsset()
	: twoSided(false), gloss(50), mainColor(Color::white), name(UNNAMED_MATERIAL) {
	material = new Material();
	mainTexels = bumpTexels = specularTexels = emissiveTexels = lightmapTexels = nullptr;
}

void MaterialAsset::ApplyAsset() {
	Shader* shader = Shader::Find("builtin/" + shaderName);
	material->SetShader(shader);

	material->SetFloat(BuiltinProperties::Gloss, gloss);

	material->SetColor(BuiltinProperties::MainColor, mainColor);
	material->SetColor(BuiltinProperties::SpecularColor, specularColor);
	material->SetColor(BuiltinProperties::EmissiveColor, emissiveColor);

	material->SetName(name);

	if (mainTexels != nullptr) {
		material->SetTexture(BuiltinProperties::MainTexture, CreateTexture2D(mainTexels).get());
	}
	else {
		material->SetTexture(BuiltinProperties::MainTexture, Texture2D::GetWhiteTexture());
	}

	if (bumpTexels != nullptr) {
		material->SetTexture(BuiltinProperties::BumpTexture, CreateTexture2D(bumpTexels).get());
	}

	if (specularTexels != nullptr) {
		material->SetTexture(BuiltinProperties::SpecularTexture, CreateTexture2D(specularTexels).get());
	}

	if (emissiveTexels != nullptr) {
		material->SetTexture(BuiltinProperties::EmissiveTexture, CreateTexture2D(emissiveTexels).get());
	}

	if (lightmapTexels != nullptr) {
		material->SetTexture(BuiltinProperties::LightmapTexture, CreateTexture2D(lightmapTexels).get());
	}
}

ref_ptr<Texture2D> MaterialAsset::CreateTexture2D(const TexelMap* texelMap) {
	ref_ptr<Texture2D> texture = new Texture2D();
	if (!texture->Create(texelMap->textureFormat, &texelMap->data[0], texelMap->colorStreamFormat, texelMap->width, texelMap->height, 4, false)) {
		return nullptr;
	}

	return texture;
}

GameObjectLoader::GameObjectLoader(const std::string& path, GameObject* root)
	: Worker(), path_(path), root_(root) {
}

GameObjectLoader::~GameObjectLoader() {
	for (TexelMapContainer::iterator ite = texelMapContainer_.begin(); ite != texelMapContainer_.end(); ++ite) {
		delete ite->second;
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

	std::string fpath = Resources::modelDirectory + path_;

	const aiScene* scene = importer.ReadFile(fpath.c_str(), flags);
	if (scene == nullptr) {
		Debug::LogError("failed to read file %s: %s", path_.c_str(), importer.GetErrorString());
		return false;
	}

	scene_ = scene;

	return true;
}

void GameObjectLoader::LoadHierarchy(GameObject* parent, aiNode* node, Mesh*& surface, SubMesh** subMeshes) {
	ref_ptr<GameObject> go = new GameObject();

	go->GetTransform()->SetParent(parent->GetTransform());
	LoadNodeTo(go.get(), node, surface, subMeshes);
	LoadChildren(go.get(), node, surface, subMeshes);
}

void GameObjectLoader::LoadNodeTo(GameObject* go, aiNode* node, Mesh*& surface, SubMesh** subMeshes) {
	go->SetName(node->mName.C_Str());

	if (go != root_) {
		Quaternion rotation;
		Vector3 translation, scale;
		AIConvert(translation, rotation, scale, node->mTransformation);

		go->GetTransform()->SetLocalScale(scale);
		go->GetTransform()->SetLocalRotation(rotation);
		go->GetTransform()->SetLocalPosition(translation);
	}

	LoadComponents(go, node, surface, subMeshes);
}

void GameObjectLoader::LoadComponents(GameObject* go, aiNode* node, Mesh*& surface, SubMesh** subMeshes) {
	if (node->mNumMeshes == 0) {
		return;
	}

	ref_ptr<Renderer> renderer;
	if (!HasAnimation()) {
		renderer = new MeshRenderer();
	}
	else {
		renderer = new SkinnedMeshRenderer();
		((SkinnedMeshRenderer*)renderer.get())->SetSkeleton(skeleton_.get());
	}

	asset_.components.push_back(std::make_pair(go, renderer));

	ref_ptr<Mesh> mesh = new Mesh();
	mesh->ShareStorage(surface);

	ref_ptr<MeshFilter> meshFilter = new MeshFilter();
	meshFilter->SetMesh(mesh.get());
	asset_.components.push_back(std::make_pair(go, meshFilter));

	for (int i = 0; i < node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		mesh->AddSubMesh(subMeshes[meshIndex]);

		uint materialIndex = scene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(asset_.materialAssets[materialIndex].material.get());
		}
	}
}

void GameObjectLoader::LoadChildren(GameObject* go, aiNode* node, Mesh*& surface, SubMesh** subMeshes) {
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

bool GameObjectLoader::LoadAttribute(MeshAsset& meshAsset, SubMesh** subMeshes) {
	ReserveMemory(meshAsset);

	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		subMeshes[i] = new SubMesh();
		TriangleBias bias{
			scene_->mMeshes[i]->mNumFaces * 3, meshAsset.indexes.size(), meshAsset.positions.size()
		};
		subMeshes[i]->SetTriangleBias(bias);
		LoadAttributeAt(i, meshAsset, subMeshes);
	}

	return true;
}

bool GameObjectLoader::LoadAttributeAt(int meshIndex, MeshAsset& meshAsset, SubMesh** subMeshes) {
	LoadVertexAttribute(meshIndex, meshAsset);
	LoadBoneAttribute(meshIndex, meshAsset, subMeshes);

	return true;
}

void GameObjectLoader::LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];

	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < aimesh->mNumVertices; ++i) {
		Vector3 pos = AIConvert(aimesh->mVertices[i]);
		Vector3 normal = AIConvert(aimesh->mNormals[i]);

		for (int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
			if (!aimesh->HasTextureCoords(j)) {
				continue;
			}

			if (j > MeshAttribute::TexCoordsCount) {
				Debug::LogWarning("only %d texture coordinates are supported.", MeshAttribute::TexCoordsCount);
				break;
			}

			const aiVector3D& v3 = aimesh->mTextureCoords[j][i];
			meshAsset.texCoords[j].push_back(Vector2(v3.x, v3.y));
		}

		if (aimesh->mTangents != nullptr) {
			const aiVector3D& v3 = aimesh->mTangents[i];
			meshAsset.tangents.push_back(Vector3(v3.x, v3.y, v3.z));
		}

		meshAsset.positions.push_back(pos);
		meshAsset.normals.push_back(normal);

		min = Vector3::Min(min, pos);
		max = Vector3::Max(max, pos);
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

void GameObjectLoader::LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh** subMeshes) {
	const aiMesh* aimesh = scene_->mMeshes[meshIndex];
	for (int i = 0; i < aimesh->mNumBones; ++i) {
		if (!skeleton_) { skeleton_ = new Skeleton(); }
		std::string name(aimesh->mBones[i]->mName.data);

		int index = skeleton_->GetBoneIndex(name);
		if (index < 0) {
			SkeletonBone bone{ name };
			AIConvert(bone.meshToBoneMatrix, aimesh->mBones[i]->mOffsetMatrix);
			index = skeleton_->GetBoneCount();
			skeleton_->AddBone(bone);
		}

		for (int j = 0; j < aimesh->mBones[i]->mNumWeights; ++j) {
			uint vertexID = subMeshes[meshIndex]->GetTriangleBias().baseVertex + aimesh->mBones[i]->mWeights[j].mVertexId;

			float weight = aimesh->mBones[i]->mWeights[j].mWeight;
			for (int k = 0; k < BlendAttribute::Quality; ++k) {
				if (Mathf::Approximately(meshAsset.blendAttrs[vertexID].weights[k], 0)) {
					meshAsset.blendAttrs[vertexID].indexes[k] = index;
					meshAsset.blendAttrs[vertexID].weights[k] = weight;

					SkeletonBone* bone = skeleton_->GetBone(index);

					Vector4 pos(meshAsset.positions[vertexID].x, meshAsset.positions[vertexID].y, meshAsset.positions[vertexID].z, 1);
					pos = bone->meshToBoneMatrix * pos;
					bone->bounds.Encapsulate(Vector3(pos.x, pos.y, pos.z));
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
		if (Mathf::Approximately(afloat, 0)) { afloat = 1; }
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

void GameObjectLoader::LoadAnimation(Animation* animation) {
	Matrix4 rootTransform;
	animation->SetRootTransform(AIConvert(rootTransform, scene_->mRootNode->mTransformation.Inverse()));

	const char* defaultClipName = nullptr;
	for (int i = 0; i < scene_->mNumAnimations; ++i) {
		aiAnimation* anim = scene_->mAnimations[i];
		std::string name = anim->mName.C_Str();

		ref_ptr<AnimationClip> clip = new AnimationClip();
		if (defaultClipName == nullptr) {
			defaultClipName = anim->mName.C_Str();
		}

		LoadAnimationClip(anim, clip.get());
		animation->AddClip(name, clip.get());
	}

	animation->SetSkeleton(skeleton_.get());
	animation->Play(defaultClipName);
}

bool GameObjectLoader::HasAnimation() {
	return skeleton_ && scene_->mNumAnimations != 0;
}

void GameObjectLoader::LoadAnimationClip(const aiAnimation* anim, AnimationClip* clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapMode::Loop);
	LoadAnimationNode(anim, scene_->mRootNode, nullptr);
}

void GameObjectLoader::LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
	const aiNodeAnim* channel = FindChannel(anim, paiNode->mName.C_Str());

	AnimationKeys keys;
	ref_ptr<AnimationCurve> curve;
	if (channel != nullptr) {
		for (int i = 0; i < channel->mNumPositionKeys; ++i) {
			const aiVectorKey& key = channel->mPositionKeys[i];
			keys.AddVector3((float)key.mTime, FrameKeyPosition, AIConvert(key.mValue));
		}

		for (int i = 0; i < channel->mNumRotationKeys; ++i) {
			const aiQuatKey& key = channel->mRotationKeys[i];
			Quaternion rotation;
			keys.AddQuaternion((float)key.mTime, FrameKeyRotation, AIConvert(rotation, key.mValue));
		}

		for (int i = 0; i < channel->mNumScalingKeys; ++i) {
			const aiVectorKey& key = channel->mScalingKeys[i];
			keys.AddVector3((float)key.mTime, FrameKeyScale, AIConvert(key.mValue));
		}

		curve = new AnimationCurve();
		curve->SetKeys(&keys);
	}

	Matrix4 matrix;
	SkeletonNode* child = skeleton_->CreateNode(paiNode->mName.C_Str(), AIConvert(matrix, paiNode->mTransformation), curve.get());
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
	TexelMap* answer = new TexelMap;

	if (String::StartsWith(name, "*")) {
		status = LoadEmbeddedTexels(*answer, String::ToInteger(name.substr(1)));
	}
	else {
		status = LoadExternalTexels(*answer, FileSystem::GetFileNameWithoutExtension(path_) + "/" + name);
	}

	if (!status) {
		delete answer;
		return false;
	}

	texelMapContainer_.insert(std::make_pair(name, answer));
	return answer;
}

bool GameObjectLoader::LoadExternalTexels(TexelMap& texelMap, const std::string& name) {
	return ImageCodec::Decode(texelMap, Resources::textureDirectory + name);
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

	SubMesh** subMeshes = nullptr;
	asset_.meshAsset.topology = MeshTopology::Triangles;

	if (scene_->mNumMeshes > 0) {
		subMeshes = new SubMesh*[scene_->mNumMeshes];
		if (!LoadAttribute(asset_.meshAsset, subMeshes)) {
			Debug::LogError("failed to load meshes for %s.", path_.c_str());
		}
	}

	if (scene_->mNumMaterials > 0) {
		asset_.materialAssets.resize(scene_->mNumMaterials);
		LoadMaterialAssets();
	}

	Mesh* surface = new Mesh();
	surface->CreateStorage();

	LoadNodeTo(root_.get(), scene_->mRootNode, surface, subMeshes);
	LoadChildren(root_.get(), scene_->mRootNode, surface, subMeshes);

	surface_ = surface;

	delete[] subMeshes;

	if (HasAnimation()) {
		ref_ptr<Animation> animation = new Animation();
		asset_.components.push_back(std::make_pair(root_.get(), animation));
		LoadAnimation(animation.get());
	}

	return true;
}

GameObject* GameObjectLoaderThreadPool::Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback) {
	ref_ptr<GameObject> root = new GameObject();
	ImportTo(root.get(), path, callback);
	return root.get();
}

bool GameObjectLoaderThreadPool::ImportTo(GameObject* go, const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback) {
	return Execute(new GameObjectLoaderWithCallback(path, go, callback));
}

void GameObjectLoaderThreadPool::OnSchedule(ZThread::Task& schedule) {
	GameObjectLoaderWithCallback* loader = (GameObjectLoaderWithCallback*)schedule.get();

	GameObjectAsset& asset = loader->GetGameObjectAsset();
	for (uint i = 0; i < asset.materialAssets.size(); ++i) {
		asset.materialAssets[i].ApplyAsset();
	}

	for (uint i = 0; i < asset.components.size(); ++i) {
		asset.components[i].first->AddComponent(asset.components[i].second.get());
	}

	loader->GetSurface()->SetAttribute(asset.meshAsset);

	GameObject* root = loader->GetGameObject();
	root->GetTransform()->SetParent(World::GetRootTransform());

	if (loader->GetParameter()) {
		(*loader->GetParameter())(root, loader->GetPath());
	}

	imported_.raise(root, loader->GetPath());
}
