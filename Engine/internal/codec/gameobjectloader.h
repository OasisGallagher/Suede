#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "image.h"
#include "engine.h"
#include "animation.h"
#include "gameobject.h"
#include "tools/noncopyable.h"
#include "gameobjectloadedlistener.h"
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

	Color mainColor;
	Color specularColor;
	Color emissiveColor;

	TexelMap* mainTexels;
	TexelMap* bumpTexels;
	TexelMap* specularTexels;
	TexelMap* emissiveTexels;
	TexelMap* lightmapTexels;
};

typedef MeshAttribute MeshAsset;

struct GameObjectAsset {
	MeshAsset meshAsset;
	std::vector<MaterialAsset> materialAssets;
};

class GameObjectLoader : public Worker, private NonCopyable {
public:
	GameObjectLoader(const std::string& path, GameObject go, WorkerEventListener* receiver);
	~GameObjectLoader();

public:
	GameObject GetGameObject() { return root_; }
	const std::string& GetPath() { return path_; }

	Mesh GetSurface() { return surface_; }
	GameObjectAsset& GetGameObjectAsset() { return asset_; }

protected:
	virtual void Run();

private:
	bool LoadAsset();
	bool Initialize(Assimp::Importer& importer);

	void LoadNodeTo(GameObject go, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);
	void LoadChildren(GameObject go, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);
	void LoadComponents(GameObject go, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);

	GameObject LoadHierarchy(GameObject parent, aiNode* node, Mesh& surface, SubMesh* subMeshes, const Bounds* boundses);

	void ReserveMemory(MeshAsset& meshAsset);
	bool LoadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes, Bounds* boundses);
	bool LoadAttributeAt(int index, MeshAsset& meshAsset, SubMesh* subMeshes, Bounds* boundses);

	void LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes);
	void LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset, Bounds* boundses);

	void LoadMaterialAssets();
	void LoadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material);

	void LoadAnimation(Animation animation);
	bool HasAnimation() { return skeleton_ && scene_->mNumAnimations != 0; }
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
	GameObjectAsset asset_;

	GameObject root_;
	std::string path_;

	Skeleton skeleton_;
	Animation animation_;
	const aiScene* scene_;

	typedef std::map<std::string, TexelMap*> TexelMapContainer;
	TexelMapContainer texelMapContainer_;
};

class GameObjectLoaderThreadPool : public ThreadPool {
public:
	GameObjectLoaderThreadPool() : ThreadPool(16), listener_(nullptr) {}

public:
	GameObject Import(const std::string& path);
	bool ImportTo(GameObject go, const std::string& path);

	void SetLoadedListener(GameObjectLoadedListener* listener);

protected:
	virtual void OnSchedule(ZThread::Task& schedule);

private:
	GameObjectLoadedListener* listener_;
};
