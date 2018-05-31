#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "image.h"
#include "entity.h"
#include "engine.h"
#include "entityimportedlistener.h"
#include "internal/async/threadpool.h"

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

class EntityAssetLoader : public AsyncWorker {
public:
	EntityAssetLoader(const std::string& path, Entity entity, AsyncEventReceiver* receiver);
	~EntityAssetLoader();

public:
	Entity GetEntity() { return root_; }
	const std::string& GetPath() { return path_; }

	Mesh GetSurface() { return surface_; }
	EntityAsset& GetEntityAsset() { return asset_; }

protected:
	virtual void OnRun();

private:
	// TODO: NonCopyable.
	EntityAssetLoader(const EntityAssetLoader&);
	const EntityAssetLoader& operator=(const EntityAssetLoader&);

private:
	bool LoadAsset();
	bool Initialize(Assimp::Importer& importer);

	void LoadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);
	void LoadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);
	void LoadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);

	Entity LoadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);

	void ReserveMemory(MeshAsset& meshAsset);
	bool LoadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes, Bounds* boundses);
	bool LoadAttributeAt(int index, MeshAsset& meshAsset, SubMesh* subMeshes, Bounds* boundses);

	void LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes);
	void LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset, Bounds* boundses);

	void LoadMaterialAssets();
	void LoadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material);

	bool LoadAnimation(Animation& animation);
	void LoadAnimationClip(const aiAnimation* anim, AnimationClip clip);
	void LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim* FindChannel(const aiAnimation* anim, const char* name);

	TexelMap* LoadTexels(const std::string& name);

	bool LoadEmbeddedTexels(TexelMap& texelMap, uint index);
	bool LoadExternalTexels(TexelMap& texelMap, const std::string& name);

private:
	static glm::vec3 AIVector3ToGLM(const aiVector3D& vec);
	static glm::mat4& AIMaterixToGLM(glm::mat4& answer, const aiMatrix4x4& mat);
	static glm::quat& AIQuaternionToGLM(glm::quat& answer, const aiQuaternion& quaternion);
	static void DecomposeAIMatrix(glm::vec3& translation, glm::quat& rotation, glm::vec3& scale, const aiMatrix4x4& mat);

private:
	Mesh surface_;
	EntityAsset asset_;

	Entity root_;
	std::string path_;

	Skeleton skeleton_;
	Animation animation_;
	const aiScene* scene_;

	typedef std::map<std::string, TexelMap*> TexelMapContainer;
	TexelMapContainer texelMapContainer_;
};

class AsyncEntityImporter : public ThreadPool {
public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

	void SetImportedListener(EntityImportedListener* listener);

protected:
	virtual void OnSchedule(ZThread::Task& schedule);

private:
	EntityImportedListener* listener_;
};
