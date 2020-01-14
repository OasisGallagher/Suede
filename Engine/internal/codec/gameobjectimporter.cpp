#include "gameobjectimporter.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "scene.h"
#include "engine.h"
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

GameObjectLoader::GameObjectLoader(GameObject* root, const std::string& path, std::function<void(GameObject*, const std::string&)> callback)
	: path_(path), root_(root), callback_(callback) {
}

GameObjectLoader::~GameObjectLoader() {
}

void GameObjectLoader::Apply() {
	if (!loaded_) {
		root_->GetScene()->DestroyGameObject(root_);
		root_ = nullptr;
	}
	else {
		for (auto& p : componentsMap_) {
			for (auto& component : p.second) {
				p.first->AddComponent(component.get());
			}
		}

		root_->GetTransform()->SetParent(root_->GetScene()->GetRootTransform());
	}

	if (callback_) { callback_(root_, path_); }
}


void GameObjectLoader::Run() {
	Assimp::Importer importer;
	if (!Initialize(importer)) {
		return;
	}

	SubMesh** subMeshes = nullptr;
	GeometryAttribute attribute;

	ref_ptr<Geometry> geometry = new Geometry();
	geometry->SetTopology(MeshTopology::Triangles);

	if (aiScene_->mNumMeshes > 0) {
		subMeshes = new SubMesh*[aiScene_->mNumMeshes];
		LoadGeometryAttribute(attribute, subMeshes);

		if (!attribute.vertices.empty()) {
			geometry->SetVertices(attribute.vertices.data(), attribute.vertices.size());
		}

		if (!attribute.normals.empty()) {
			geometry->SetNormals(attribute.normals.data(), attribute.normals.size());
		}

		for (int i = 0; i < Geometry::TexCoordsCount; ++i) {
			if (!attribute.texCoords[i].empty()) {
				geometry->SetTexCoords(i, attribute.texCoords[i].data(), attribute.texCoords[i].size());
			}
		}

		if (!attribute.tangents.empty()) {
			geometry->SetTangents(attribute.tangents.data(), attribute.tangents.size());
		}

		if (!attribute.blendAttrs.empty()) {
			geometry->SetBlendAttributes(attribute.blendAttrs.data(), attribute.blendAttrs.size());
		}

		if (!attribute.indexes.empty()) {
			geometry->SetIndexes(attribute.indexes.data(), attribute.indexes.size());
		}
	}

	std::vector<ref_ptr<Material>> materials(aiScene_->mNumMaterials);
	if (aiScene_->mNumMaterials > 0) {
		for (int i = 0; i < aiScene_->mNumMaterials; ++i) {
			materials[i] = new Material();
		}

		LoadMaterials(materials);
	}

	LoadNodeTo(root_, aiScene_->mRootNode, materials, geometry.get(), subMeshes);
	LoadChildren(root_, aiScene_->mRootNode, materials, geometry.get(), subMeshes);

	delete[] subMeshes;

	if (HasAnimation()) {
		ref_ptr<Animation> animation = new Animation();
		componentsMap_[root_].push_back(animation);
		LoadAnimation(animation.get());
	}

	loaded_ = true;
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

	aiScene_ = scene;

	return true;
}

void GameObjectLoader::LoadHierarchy(GameObject* parent, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes) {
	ref_ptr<GameObject> go = new GameObject();

	go->GetTransform()->SetParent(parent->GetTransform());
	LoadNodeTo(go.get(), node, materials, geometry, subMeshes);
	LoadChildren(go.get(), node, materials, geometry, subMeshes);
}

void GameObjectLoader::LoadNodeTo(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes) {
	go->SetName(node->mName.C_Str());

	if (go != root_) {
		Quaternion rotation;
		Vector3 translation, scale;
		AIConvert(translation, rotation, scale, node->mTransformation);

		go->GetTransform()->SetLocalScale(scale);
		go->GetTransform()->SetLocalRotation(rotation);
		go->GetTransform()->SetLocalPosition(translation);
	}

	LoadComponents(go, node, materials, geometry, subMeshes);
}

void GameObjectLoader::LoadComponents(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes) {
	if (node->mNumMeshes == 0) {
		return;
	}

	Renderer* renderer;
	if (!HasAnimation()) {
		renderer = new MeshRenderer();
	}
	else {
		renderer = new SkinnedMeshRenderer();
		((SkinnedMeshRenderer*)renderer)->SetSkeleton(skeleton_.get());
	}

	componentsMap_[go].push_back(renderer);

	ref_ptr<Mesh> mesh = new Mesh();
	mesh->SetGeometry(geometry);

	ref_ptr<MeshFilter> meshFilter = new MeshFilter();
	meshFilter->SetMesh(mesh.get());
	componentsMap_[go].push_back(meshFilter);

	for (int i = 0; i < node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		mesh->AddSubMesh(subMeshes[meshIndex]);

		uint materialIndex = aiScene_->mMeshes[meshIndex]->mMaterialIndex;
		if (materialIndex < aiScene_->mNumMaterials) {
			renderer->AddMaterial(materials[materialIndex].get());
		}
	}
}

void GameObjectLoader::LoadChildren(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes) {
	for (int i = 0; i < node->mNumChildren; ++i) {
		LoadHierarchy(go, node->mChildren[i], materials, geometry, subMeshes);
	}
}

void GameObjectLoader::ReserveMemory(GeometryAttribute& attribute) {
	uint indexCount = 0, vertexCount = 0;
	for (int i = 0; i < aiScene_->mNumMeshes; ++i) {
		indexCount += aiScene_->mMeshes[i]->mNumFaces * 3;
		vertexCount += aiScene_->mMeshes[i]->mNumVertices;
	}

	attribute.vertices.reserve(vertexCount);
	attribute.normals.reserve(vertexCount);

	for (int i = 0; i < Geometry::TexCoordsCount; ++i) {
		attribute.texCoords[i].reserve(vertexCount);
	}

	attribute.tangents.reserve(vertexCount);
	attribute.indexes.reserve(indexCount);
	attribute.blendAttrs.resize(vertexCount);
}

void GameObjectLoader::LoadGeometryAttribute(GeometryAttribute& attribute, SubMesh** subMeshes) {
	ReserveMemory(attribute);

	for (int i = 0; i < aiScene_->mNumMeshes; ++i) {
		subMeshes[i] = new SubMesh();
		TriangleBias bias{
			aiScene_->mMeshes[i]->mNumFaces * 3, attribute.indexes.size(), attribute.vertices.size()
		};
		subMeshes[i]->SetTriangleBias(bias);
		LoadAttributeAt(i, attribute, subMeshes);
	}
}

void GameObjectLoader::LoadAttributeAt(int meshIndex, GeometryAttribute& attribute, SubMesh** subMeshes) {
	LoadVertexAttribute(meshIndex, attribute);
	LoadBoneAttribute(meshIndex, attribute, subMeshes);
}

void GameObjectLoader::LoadVertexAttribute(int meshIndex, GeometryAttribute& attribute) {
	const aiMesh* aimesh = aiScene_->mMeshes[meshIndex];

	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < aimesh->mNumVertices; ++i) {
		Vector3 pos = AIConvert(aimesh->mVertices[i]);
		Vector3 normal = AIConvert(aimesh->mNormals[i]);

		for (int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
			if (!aimesh->HasTextureCoords(j)) {
				continue;
			}

			if (j > Geometry::TexCoordsCount) {
				Debug::LogWarning("only %d texture coordinates are supported.", Geometry::TexCoordsCount);
				break;
			}

			const aiVector3D& v3 = aimesh->mTextureCoords[j][i];
			attribute.texCoords[j].push_back(Vector2(v3.x, v3.y));
		}

		if (aimesh->mTangents != nullptr) {
			const aiVector3D& v3 = aimesh->mTangents[i];
			attribute.tangents.push_back(Vector3(v3.x, v3.y, v3.z));
		}

		attribute.vertices.push_back(pos);
		attribute.normals.push_back(normal);

		min = Vector3::Min(min, pos);
		max = Vector3::Max(max, pos);
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

void GameObjectLoader::LoadBoneAttribute(int meshIndex, GeometryAttribute& attribute, SubMesh** subMeshes) {
	const aiMesh* aimesh = aiScene_->mMeshes[meshIndex];
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

		Vector3 aabbMin, aabbMax;
		SkeletonBone* bone = skeleton_->GetBone(index);
		for (int j = 0; j < aimesh->mBones[i]->mNumWeights; ++j) {
			uint vertexID = subMeshes[meshIndex]->GetTriangleBias().baseVertex + aimesh->mBones[i]->mWeights[j].mVertexId;

			float weight = aimesh->mBones[i]->mWeights[j].mWeight;
			for (int k = 0; k < BlendAttribute::Quality; ++k) {
				if (Mathf::Approximately(attribute.blendAttrs[vertexID].weights[k], 0)) {
					attribute.blendAttrs[vertexID].indexes[k] = index;
					attribute.blendAttrs[vertexID].weights[k] = weight;

					Vector4 pos(attribute.vertices[vertexID].x, attribute.vertices[vertexID].y, attribute.vertices[vertexID].z, 1);
					pos = bone->meshToBoneMatrix * pos;
					aabbMin = Vector3::Min(Vector3(pos.x, pos.y, pos.z), aabbMin);
					aabbMax = Vector3::Max(Vector3(pos.x, pos.y, pos.z), aabbMin);
					break;
				}
			}
		}

		bone->bounds.SetMinMax(aabbMin, aabbMax);
	}
}

void GameObjectLoader::LoadMaterials(std::vector<ref_ptr<Material>>& materials) {
	for (int i = 0; i < aiScene_->mNumMaterials; ++i) {
		LoadMaterial(materials[i].get(), aiScene_->mMaterials[i]);
	}
}

void GameObjectLoader::LoadMaterial(Material* material, aiMaterial* resource) {
	int aint;
	float afloat;
	aiString astring;
	aiColor3D acolor;

	material->SetShader(Shader::Find(HasAnimation() ? "builtin/lit_animated_texture" : "builtin/lit_texture"));

	if (resource->Get(AI_MATKEY_NAME, astring) == AI_SUCCESS) {
		material->SetName(FileSystem::GetFileName(astring.C_Str()));
	}
	else {
		material->SetName(UNNAMED_MATERIAL);
	}

	if (resource->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), astring) == AI_SUCCESS) {
		material->SetTexture(BuiltinProperties::MainTexture, LoadTexture(FileSystem::GetFileName(astring.C_Str())));
	}
	else {
		material->SetTexture(BuiltinProperties::MainTexture, Texture2D::GetWhiteTexture());
	}

	if (resource->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), astring) == AI_SUCCESS) {
		material->SetTexture(BuiltinProperties::BumpTexture, LoadTexture(FileSystem::GetFileName(astring.C_Str())));
	}

	if (resource->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), astring) == AI_SUCCESS) {
		material->SetTexture(BuiltinProperties::SpecularTexture, LoadTexture(FileSystem::GetFileName(astring.C_Str())));
	}

	if (resource->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), astring) == AI_SUCCESS) {
		material->SetTexture(BuiltinProperties::LightmapTexture, LoadTexture(FileSystem::GetFileName(astring.C_Str())));
	}

	if (resource->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), astring) == AI_SUCCESS) {
		material->SetTexture(BuiltinProperties::EmissiveTexture, LoadTexture(FileSystem::GetFileName(astring.C_Str())));
	}

	Color color = Color::white;
	if (resource->Get(AI_MATKEY_OPACITY, afloat) == AI_SUCCESS) {
		if (Mathf::Approximately(afloat, 0)) { afloat = 1; }
		color.a = afloat;
	}

	if (resource->Get(AI_MATKEY_COLOR_DIFFUSE, acolor) == AI_SUCCESS) {
		color = Color(acolor.r, acolor.g, acolor.b, color.a);
	}

	material->SetColor(BuiltinProperties::MainColor, color);

	color = Color::black;
	if (resource->Get(AI_MATKEY_COLOR_SPECULAR, acolor) == AI_SUCCESS) {
		color = Color(acolor.r, acolor.g, acolor.b, 1);
	}

	material->SetColor(BuiltinProperties::SpecularColor, color);

	color = Color::black;
	if (resource->Get(AI_MATKEY_COLOR_EMISSIVE, acolor) == AI_SUCCESS) {
		color = Color(acolor.r, acolor.g, acolor.b, 1);
	}

	material->SetColor(BuiltinProperties::EmissiveColor, color);

	float gloss = 50;
	if (resource->Get(AI_MATKEY_SHININESS, afloat) == AI_SUCCESS) {
		gloss = afloat;
	}

	material->SetFloat(BuiltinProperties::Gloss, gloss);

	bool twoSided = false;
	if (resource->Get(AI_MATKEY_TWOSIDED, aint) == AI_SUCCESS) {
		twoSided = !!aint;
	}
}

void GameObjectLoader::LoadAnimation(Animation* animation) {
	Matrix4 rootTransform;
	animation->SetRootTransform(AIConvert(rootTransform, aiScene_->mRootNode->mTransformation.Inverse()));

	const char* defaultClipName = nullptr;
	for (int i = 0; i < aiScene_->mNumAnimations; ++i) {
		aiAnimation* anim = aiScene_->mAnimations[i];
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
	return skeleton_ && aiScene_->mNumAnimations != 0;
}

void GameObjectLoader::LoadAnimationClip(const aiAnimation* anim, AnimationClip* clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapMode::Loop);
	LoadAnimationNode(anim, aiScene_->mRootNode, nullptr);
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

Texture2D* GameObjectLoader::LoadTexture(const std::string& name) {
	auto pos = rawImages_.find(name);
	if (pos != rawImages_.end()) {
		return pos->second.get();
	}

	RawImage texels;
	ref_ptr<Texture2D> texture;
	if (String::StartsWith(name, "*")) {
		if (LoadEmbeddedTexels(texels, String::ToInteger(name.substr(1)))) {
			texture = new Texture2D();
		}
	}
	else if (LoadExternalTexels(texels, FileSystem::GetFileNameWithoutExtension(path_) + "/" + name)) {
		texture = new Texture2D();
	}

	if (texture && !texture->Create(texels.textureFormat, texels.pixels.data(), texels.colorStreamFormat, texels.width, texels.height, 4, false)) {
		texture = nullptr;
	}
	
	if (!texture) {
		texture = Texture2D::GetWhiteTexture();
	}

	rawImages_.insert(std::make_pair(name, texture));
	return texture.get();
}

bool GameObjectLoader::LoadExternalTexels(RawImage& rawImage, const std::string& name) {
	return ImageCodec::Decode(rawImage, Resources::textureDirectory + name);
}

bool GameObjectLoader::LoadEmbeddedTexels(RawImage& rawImage, uint index) {
	SUEDE_ASSERT(index < aiScene_->mNumTextures);

	aiTexture* aitex = aiScene_->mTextures[index];
	if (aitex->mHeight == 0) {
		if (!ImageCodec::Decode(rawImage, aitex->pcData, aitex->mWidth)) {
			return false;
		}
	}
	else {
		rawImage.textureFormat = TextureFormat::Rgba;
		rawImage.pixels.assign((uchar*)aitex->pcData, (uchar*)aitex->pcData + aitex->mWidth * aitex->mHeight * sizeof(aiTexel));
		rawImage.colorStreamFormat = ColorStreamFormat::Argb;
		rawImage.width = aitex->mWidth;
		rawImage.height = aitex->mHeight;
	}

	return true;
}

ref_ptr<GameObject> GameObjectImporter::Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback) {
	ref_ptr<GameObject> root = new GameObject();
	AddTask(new GameObjectLoader(root.get(), path, callback));
	return root;
}

void GameObjectImporter::OnSchedule(Task* task) {
	((GameObjectLoader*)task)->Apply();
}
