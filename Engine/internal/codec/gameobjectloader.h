#pragma once

#include "lua++.h"

#include "mesh.h"
#include "image.h"
#include "material.h"
#include "animation.h"
#include "gameobject.h"
#include "tools/event.h"
#include "tools/noncopyable.h"
#include "internal/async/threadpool.h"

struct aiNode;
struct aiScene;
struct aiMaterial;
struct aiNodeAnim;
struct aiAnimation;

namespace Assimp { class Importer; }

struct MaterialAsset {
	MaterialAsset();
	~MaterialAsset() {}

	void ApplyAsset();
	ref_ptr<Texture2D> CreateTexture2D(const TexelMap* texelMap);

	ref_ptr<Material> material;

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
	std::vector<std::pair<GameObject*, ref_ptr<Component>>> components;
};

class GameObjectLoader : public Task, private NonCopyable {
public:
	GameObjectLoader(const std::string& path, GameObject* root);
	~GameObjectLoader();

public:
	GameObject* GetGameObject() { return root_.get(); }
	const std::string& GetPath() { return path_; }

	Mesh* GetSurface() { return surface_.get(); }
	GameObjectAsset& GetGameObjectAsset() { return asset_; }

public:
	virtual void Run();

private:
	bool LoadAsset();
	bool Initialize(Assimp::Importer& importer);

	void LoadNodeTo(GameObject* go, aiNode* node, Mesh*& surface, SubMesh** subMeshes);
	void LoadChildren(GameObject* go, aiNode* node, Mesh*& surface, SubMesh** subMeshes);
	void LoadComponents(GameObject* go, aiNode* node, Mesh*& surface, SubMesh** subMeshes);

	void LoadHierarchy(GameObject* parent, aiNode* node, Mesh*& surface, SubMesh** subMeshes);

	void ReserveMemory(MeshAsset& meshAsset);
	bool LoadAttribute(MeshAsset& meshAsset, SubMesh** subMeshes);
	bool LoadAttributeAt(int index, MeshAsset& meshAsset, SubMesh** subMeshes);

	void LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh** subMeshes);
	void LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset);

	void LoadMaterialAssets();
	void LoadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material);

	bool HasAnimation();
	void LoadAnimation(Animation* animation);
	void LoadAnimationClip(const aiAnimation* anim, AnimationClip* clip);
	void LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim* FindChannel(const aiAnimation* anim, const char* name);

	TexelMap* LoadTexels(const std::string& name);

	bool LoadEmbeddedTexels(TexelMap& texelMap, uint index);
	bool LoadExternalTexels(TexelMap& texelMap, const std::string& name);

private:
	ref_ptr<Mesh> surface_;
	GameObjectAsset asset_;

	std::string path_;
	ref_ptr<GameObject> root_;

	ref_ptr<Skeleton> skeleton_;
	const aiScene* scene_;

	typedef std::map<std::string, TexelMap*> TexelMapContainer;
	TexelMapContainer texelMapContainer_;
};

template <class T>
class GameObjectLoaderParameterized : public GameObjectLoader {
public:
public:
	GameObjectLoaderParameterized(const std::string& path, GameObject* root, const T& value) 
		: GameObjectLoader(path, root), parameter_(value) {
	}

public:
	T& GetParameter() { return parameter_; }

private:
	T parameter_;
};

typedef GameObjectLoaderParameterized<Lua::Func<void, GameObject*, const std::string&>> GameObjectLoaderWithCallback;

class GameObjectLoaderThreadPool : public ThreadPool {
public:
	GameObjectLoaderThreadPool(event<GameObject*, const std::string&>& imported) : ThreadPool(std::thread::hardware_concurrency()), imported_(imported) {}
	~GameObjectLoaderThreadPool() {}

public:
	ref_ptr<GameObject> Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback);
	void ImportTo(GameObject* go, const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback);

protected:
	virtual void OnSchedule(Task* task);

private:
	event<GameObject*, const std::string&>& imported_;
};
