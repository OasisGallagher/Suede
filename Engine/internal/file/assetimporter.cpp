#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/config.h>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "tools/path.h"
#include "tools/debug.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "assetimporter.h"
#include "internal/memory/memory.h"
#include "internal/memory/factory.h"
#include "internal/base/shaderinternal.h"
#include "internal/world/worldinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"
#include "internal/base/animationinternal.h"
#include "internal/sprites/spriteinternal.h"

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

static const int DEFAULT_ALBEDO = 0xffffff;

Sprite AssetImporter::Import(const std::string& path) {
	Sprite sprite = CREATE_OBJECT(Sprite);
	ImportTo(sprite, path);
	return sprite;
}

bool AssetImporter::ImportTo(Sprite sprite, const std::string& path) {
	Assert(sprite);

	Assimp::Importer importer;
	Initialize(path, importer);

	Surface* surfaces = nullptr;
	Material* materials = nullptr;

	if (scene_->mNumMaterials > 0) {
		materials = Memory::CreateArray<Material>(scene_->mNumMaterials);
		if (!ReadMaterials(materials)) {
			Debug::LogError("failed to load materials for " + path);
		}
	}

	if (scene_->mNumMeshes > 0) {
		surfaces = Memory::CreateArray<Surface>(scene_->mNumMeshes);
		if (!ReadSurfaces(surfaces)) {
			Debug::LogError("failed to load surfaces for " + path);
		}
	}

	ReadHierarchy(sprite, scene_->mRootNode, surfaces, materials);

	Memory::ReleaseArray(surfaces);
	Memory::ReleaseArray(materials);

	Animation animation;
	if (ReadAnimation(animation)) {
		sprite->SetAnimation(animation);
	}

	return true;
}

Surface AssetImporter::ImportSurface(const std::string& path) {
	Assimp::Importer importer;
	Initialize(path, importer);
	if (scene_->mNumMeshes == 0) {
		return nullptr;
	}

	Surface surface = CREATE_OBJECT(Surface);
	if (!ReadSurface(surface, 0)) {
		Debug::LogError("failed to load surface for " + path);
	}

	return surface;
}

void AssetImporter::Initialize(const std::string& path, Assimp::Importer &importer) {
	unsigned flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
		| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;

	if (String::EndsWith(path, ".fbx")) {
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	}

	std::string fpath = Path::GetResourceRootDirectory() + path;

	const aiScene* scene = importer.ReadFile(fpath.c_str(), flags);
	AssertX(scene != nullptr, "failed to read file " + fpath + ": " + importer.GetErrorString());
	Clear();

	path_ = fpath;
	scene_ = scene;
}

void AssetImporter::Clear() {
	path_.clear();
	scene_ = nullptr;
	skeleton_.reset();
	animation_.reset();
}

Sprite AssetImporter::ReadHierarchy(Sprite parent, aiNode* node, Surface* surfaces, Material* materials) {
	Sprite sprite = dsp_cast<Sprite>(worldInstance->Create(ObjectTypeSprite));
	sprite->SetName(node->mName.C_Str());
	sprite->SetParent(parent);

	glm::vec3 translation, scale;
	glm::quat rotation;
	DecomposeAIMatrix(translation, rotation, scale, node->mTransformation);

	sprite->SetLocalScale(scale);
	sprite->SetLocalRotation(rotation);
	sprite->SetLocalPosition(translation);

	Renderer renderer;

	for (int i = 0; i < node->mNumMeshes; ++i) {
		sprite->AddSurface(surfaces[node->mMeshes[i]]);
		
		if (!renderer) {
			renderer = dsp_cast<SurfaceRenderer>(worldInstance->Create(ObjectTypeSurfaceRenderer));
		}

		unsigned materialIndex = scene_->mMeshes[node->mMeshes[i]]->mMaterialIndex;
		if (materialIndex < scene_->mNumMaterials) {
			renderer->AddMaterial(materials[materialIndex]);
		}
	}

	sprite->SetRenderer(renderer);

	for (int i = 0; i < node->mNumChildren; ++i) {
		ReadHierarchy(sprite, node->mChildren[i], surfaces, materials);
	}

	return sprite;
}

bool AssetImporter::ReadSurfaces(Surface* surfaces) {
	for (int i = 0; i < scene_->mNumMeshes; ++i) {
		Surface surface = CREATE_OBJECT(Surface);
		ReadSurface(surface, i);
		surfaces[i] = surface;
	}

	return true;
}

bool AssetImporter::ReadSurface(Surface surface, int index) {
	SurfaceAttribute attribute;
	Mesh mesh = CREATE_OBJECT(Mesh);
	mesh->SetTriangles(scene_->mMeshes[index]->mNumFaces * 3, 0, 0);
	surface->AddMesh(mesh);

	ReadSurfaceAttributes(surface, index, attribute);
	surface->SetAttribute(attribute);

	return true;
}

void AssetImporter::ReadSurfaceAttributes(Surface surface, int index, SurfaceAttribute& attribute) {
	int indexCount = scene_->mMeshes[index]->mNumFaces * 3;
	int vertexCount = scene_->mMeshes[index]->mNumVertices;
	attribute.positions.reserve(vertexCount);
	attribute.normals.reserve(vertexCount);
	attribute.texCoords.reserve(vertexCount);
	attribute.tangents.reserve(vertexCount);
	attribute.indexes.reserve(indexCount);
	attribute.blendAttrs.resize(vertexCount);

	for (int i = 0; i < vertexCount; ++i) {
		memset(&attribute.blendAttrs[i], 0, sizeof(BlendAttribute));
	}

	ReadVertexAttributes(index, attribute);
	ReadBoneAttributes(index, attribute);
}

void AssetImporter::ReadVertexAttributes(int index, SurfaceAttribute& attribute) {
	const aiMesh* aimesh = scene_->mMeshes[index];

	const aiVector3D zero(0);
	for (unsigned i = 0; i < aimesh->mNumVertices; ++i) {
		const aiVector3D* pos = &aimesh->mVertices[i];
		const aiVector3D* normal = &aimesh->mNormals[i];
		const aiVector3D* texCoord = aimesh->HasTextureCoords(0) ? &(aimesh->mTextureCoords[0][i]) : &zero;
		const aiVector3D* tangent = (aimesh->mTangents != nullptr) ? &aimesh->mTangents[i] : &zero;

		attribute.positions.push_back(glm::vec3(pos->x, pos->y, pos->z));
		attribute.normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
		attribute.texCoords.push_back(glm::vec2(texCoord->x, texCoord->y));
		attribute.tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));
	}

	for (unsigned i = 0; i < aimesh->mNumFaces; ++i) {
		const aiFace& face = aimesh->mFaces[i];
		AssertX(face.mNumIndices == 3, "invalid index count");
		attribute.indexes.push_back(face.mIndices[0]);
		attribute.indexes.push_back(face.mIndices[1]);
		attribute.indexes.push_back(face.mIndices[2]);
	}
}

void AssetImporter::ReadBoneAttributes(int index, SurfaceAttribute& attribute) {
	const aiMesh* aimesh = scene_->mMeshes[index];
	for (int i = 0; i < aimesh->mNumBones; ++i) {
		if (!skeleton_) { skeleton_ = CREATE_OBJECT(Skeleton); }
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
			unsigned vertexID = aimesh->mBones[i]->mWeights[j].mVertexId;

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
		Material material = CREATE_OBJECT(Material);
		ReadMaterial(material, i);
		materials[i] = material;
	}

	return true;
}

bool AssetImporter::ReadMaterial(Material material, int index) {
	const aiMaterial* aiMat = scene_->mMaterials[index];

	std::string shaderName = "lit_texture";
	if (scene_->mNumAnimations != 0) {
		shaderName = "lit_animated_texture";
	}

	material->SetRenderState(Cull, Off);
	material->SetRenderState(DepthTest, LessEqual);

	Shader shader = CREATE_OBJECT(Shader);
	shader->Load("buildin/shaders/" + shaderName);
	material->SetShader(shader);

	// TODO: Test.
	int texCount = aiMat->GetTextureCount(aiTextureType_DIFFUSE);
	aiString dpath;
	std::string prefix = "textures/";
	bool hasDiffuseTexture = false;
	Texture2D texture = CREATE_OBJECT(Texture2D);

	if (texCount > 0 && aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &dpath) == AI_SUCCESS) {
		if (texture->Load(prefix + dpath.data)) {
			hasDiffuseTexture = true;
		}
	}

	if (!hasDiffuseTexture) {
		texture->Load(&DEFAULT_ALBEDO, 1, 1);
	}

	material->SetTexture(Variables::mainTexture, texture);

	return true;
}

bool AssetImporter::ReadAnimation(Animation& animation) {
	if (scene_->mNumAnimations == 0) {
		return true;
	}
	
	animation = CREATE_OBJECT(Animation);

	glm::mat4 rootTransform;
	animation->SetRootTransform(AIMaterixToGLM(rootTransform, scene_->mRootNode->mTransformation.Inverse()));

	const char* defaultClipName = nullptr;
	for (int i = 0; i < scene_->mNumAnimations; ++i) {
		aiAnimation* anim = scene_->mAnimations[i];
		std::string name = anim->mName.C_Str();

		AnimationClip clip = CREATE_OBJECT(AnimationClip);
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
	AnimationKeys keys = CREATE_OBJECT(AnimationKeys);
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

		curve = CREATE_OBJECT(AnimationCurve);
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

bool AssetImporter::InitRenderer(Renderer renderer) {
	std::string shaderName = "lit_texture";
	if (scene_->mNumAnimations == 0) {
		renderer = CREATE_OBJECT(SurfaceRenderer);
	}
	else {
		SkinnedSurfaceRenderer skinnedSurfaceRenderer;
		renderer = skinnedSurfaceRenderer = CREATE_OBJECT(SkinnedSurfaceRenderer);
		skinnedSurfaceRenderer->SetSkeleton(skeleton_);
	}

	return true;
}
