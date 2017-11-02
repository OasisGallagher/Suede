#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "tools/path.h"
#include "tools/debug.h"
#include "tools/mathf.h"
#include "modelimporter.h"
#include "internal/memory/memory.h"
#include "internal/memory/factory.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"
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

static glm::vec3& AIVector3ToGLM(glm::vec3& answer, const aiVector3D& vec) {
	answer = glm::vec3(vec.x, vec.y, vec.z);
	return answer;
}

bool ModelImporter::Import(const std::string& path, int mask) {
	Assimp::Importer importer;
	unsigned flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices 
		| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs;

	std::string fpath = Path::GetResourceRootDirectory() + path;
	const aiScene* scene = importer.ReadFile(fpath.c_str(), flags);

	AssertX(scene != nullptr, "failed to read file " + fpath + ": " + importer.GetErrorString());
	Clear();

	path_ = fpath;
	scene_ = scene;

	if (!ImportSurface(surface_)) {
		return false;
	}

	if ((mask & MaskImportAnimation) != 0 && !ImportAnimation(animation_)) {
		return false;
	}

	if ((mask & MaskCreateRenderer) != 0) {
		return InitRenderer(animation_, renderer_);
	}

	return true;
}

void ModelImporter::Clear() {
	path_.clear();
	scene_ = nullptr;
	surface_.reset();
	renderer_.reset();
	skeleton_.reset();
	animation_.reset();
}

bool ModelImporter::ImportSurface(Surface& surface) {
	surface = CREATE_OBJECT(Surface);

	MaterialTextures* textures = Memory::CreateArray<MaterialTextures>(scene_->mNumMaterials);
	ImportTextures(textures);

	SurfaceAttribute attribute;
	ImportSurfaceAttributes(surface, attribute, textures);
	surface->SetAttribute(attribute);

	Memory::ReleaseArray(textures);
	return true;
}

void ModelImporter::ImportTextures(MaterialTextures* textures) {
	std::string dir = Path::GetDirectory(path_);

	for (unsigned i = 0; i < scene_->mNumMaterials; ++i) {
		const aiMaterial* mat = scene_->mMaterials[i];

		ImportTexture(mat, textures[i].bump, aiTextureType_NORMALS);
		ImportTexture(mat, textures[i].albedo, aiTextureType_DIFFUSE);
		ImportTexture(mat, textures[i].specular, aiTextureType_SPECULAR);
	}
}

void ModelImporter::ImportTexture(const aiMaterial* mat, Texture& dest, int textureType) {
	aiString dpath;
	std::string prefix = "textures/";
	if (mat->GetTextureCount((aiTextureType)textureType) > 0 && mat->GetTexture((aiTextureType)textureType, 0, &dpath) == AI_SUCCESS) {
		Texture2D texture = CREATE_OBJECT(Texture2D);
		if (texture->Load(prefix + dpath.data)) {
			dest = texture;
		}
	}
}

void ModelImporter::ImportSurfaceAttributes(Surface surface, SurfaceAttribute& attribute, MaterialTextures* textures) {
	MeshSize size;
	ImportMeshes(surface, textures, size);

	attribute.positions.reserve(size.vertexCount);
	attribute.normals.reserve(size.vertexCount);
	attribute.texCoords.reserve(size.vertexCount);
	attribute.tangents.reserve(size.vertexCount);
	attribute.indexes.reserve(size.indexCount);
	attribute.blendAttrs.resize(size.vertexCount);

	for (int i = 0; i < size.vertexCount; ++i) {
		memset(&attribute.blendAttrs[i], 0, sizeof(BlendAttribute));
	}

	for (unsigned i = 0; i < scene_->mNumMeshes; ++i) {
		ImportMeshAttributes(scene_->mMeshes[i], i, attribute);
		ImportBoneAttributes(scene_->mMeshes[i], i, surface, attribute);
	}
}

void ModelImporter::ImportMeshes(Surface surface, MaterialTextures* textures, MeshSize& size) {
	unsigned vertexCount = 0, indexCount = 0;
	for (unsigned i = 0; i < scene_->mNumMeshes; ++i) {
		Mesh mesh = CREATE_OBJECT(Mesh);
		mesh->SetTriangles(scene_->mMeshes[i]->mNumFaces * 3, vertexCount, indexCount);

		if (scene_->mMeshes[i]->mMaterialIndex < scene_->mNumMaterials) {
			mesh->SetMaterialTextures(textures[scene_->mMeshes[i]->mMaterialIndex]);
		}

		vertexCount += scene_->mMeshes[i]->mNumVertices;
		indexCount += scene_->mMeshes[i]->mNumFaces * 3;

		surface->AddMesh(mesh);
	}

	size.vertexCount = vertexCount;
	size.indexCount = indexCount;
}

void ModelImporter::ImportMeshAttributes(const aiMesh* aimesh, int nm, SurfaceAttribute& attribute) {
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

void ModelImporter::ImportBoneAttributes(const aiMesh* aimesh, int nm, Surface surface, SurfaceAttribute& attribute) {
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
			unsigned vertexCount, baseVertex, baseIndex;
			surface->GetMesh(nm)->GetTriangles(vertexCount, baseVertex, baseIndex);
			unsigned vertexID = baseVertex + aimesh->mBones[i]->mWeights[j].mVertexId;
			float weight = aimesh->mBones[i]->mWeights[j].mWeight;
			for (int k = 0; k < BlendAttribute::Quality; ++k) {
				if (Mathf::Approximately(attribute.blendAttrs[vertexID].weights[k])) {
					attribute.blendAttrs[vertexID].indexes[k] = index;
					attribute.blendAttrs[vertexID].weights[k] = weight;
					break;
				}
			}
		}
	}
}

bool ModelImporter::ImportAnimation(Animation& animation) {
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

		ImportAnimationClip(anim, clip);
		animation->AddClip(name, clip);
	}

	animation->SetSkeleton(skeleton_);
	animation->Play(defaultClipName);

	return true;
}

void ModelImporter::ImportAnimationClip(const aiAnimation* anim, AnimationClip clip) {
	clip->SetTicksPerSecond((float)anim->mTicksPerSecond);
	clip->SetDuration((float)anim->mDuration);
	clip->SetWrapMode(AnimationWrapModeLoop);
	ImportAnimationNode(anim, scene_->mRootNode, nullptr);
}

void ModelImporter::ImportAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode) {
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
		ImportAnimationNode(anim, paiNode->mChildren[i], child);
	}
}

const aiNodeAnim* ModelImporter::FindChannel(const aiAnimation* anim, const char* name) {
	for (int i = 0; i < anim->mNumChannels; ++i) {
		if (strcmp(anim->mChannels[i]->mNodeName.C_Str(), name) == 0) {
			return anim->mChannels[i];
		}
	}

	return nullptr;
}

bool ModelImporter::InitRenderer(Animation animation, Renderer& renderer) {
	std::string shaderName = "lit_texture";
	if (!animation) {
		renderer = CREATE_OBJECT(SurfaceRenderer);
	}
	else {
		SkinnedSurfaceRenderer skinnedSurfaceRenderer;
		renderer = skinnedSurfaceRenderer = CREATE_OBJECT(SkinnedSurfaceRenderer);
		skinnedSurfaceRenderer->SetSkeleton(skeleton_);
		shaderName = "lit_animated_texture";
	}

	renderer->SetRenderState(Cull, Off);
	renderer->SetRenderState(DepthTest, LessEqual);

	Shader shader = CREATE_OBJECT(Shader);
	shader->Load("buildin/shaders/" + shaderName);

	Material material = CREATE_OBJECT(Material);
	material->SetShader(shader);
	renderer->AddMaterial(material);

	return true;
}
