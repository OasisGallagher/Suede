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

struct GameObjectAsset {
	std::vector<std::pair<GameObject*, ref_ptr<Component>>> components;
	void Apply() {
		for (auto& pair : components) {
			pair.first->AddComponent(pair.second.get());
		}
	}
};

class GameObjectLoader : public Task, private NonCopyable {
public:
	GameObjectLoader(GameObject* root, const std::string& path, std::function<void(GameObject*, const std::string&)> callback);
	~GameObjectLoader();

public:
	GameObject* GetGameObject() { return root_.get(); }
	const std::string& GetPath() { return path_; }

	Mesh* GetSurface() { return surface_.get(); }
	GameObjectAsset& GetGameObjectAsset() { return asset_; }

	void InvokeCallback() { if (callback_) { callback_(root_.get(), path_); } }

public:
	virtual void Run();

private:
	bool LoadAsset();
	bool Initialize(Assimp::Importer& importer);

	void LoadNodeTo(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Mesh*& surface, SubMesh** subMeshes);
	void LoadChildren(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Mesh*& surface, SubMesh** subMeshes);
	void LoadComponents(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Mesh*& surface, SubMesh** subMeshes);

	void LoadHierarchy(GameObject* parent, aiNode* node, std::vector<ref_ptr<Material>>& materials, Mesh*& surface, SubMesh** subMeshes);

	void ReserveMemory(MeshAttribute& attribute);
	bool LoadAttribute(MeshAttribute& attribute, SubMesh** subMeshes);
	bool LoadAttributeAt(int index, MeshAttribute& attribute, SubMesh** subMeshes);

	void LoadBoneAttribute(int meshIndex, MeshAttribute& attribute, SubMesh** subMeshes);
	void LoadVertexAttribute(int meshIndex, MeshAttribute& attribute);

	void LoadMaterials(std::vector<ref_ptr<Material>>& materials);
	void LoadMaterial(Material* material, aiMaterial* resource);

	bool HasAnimation();
	void LoadAnimation(Animation* animation);
	void LoadAnimationClip(const aiAnimation* anim, AnimationClip* clip);
	void LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim* FindChannel(const aiAnimation* anim, const char* name);

	Texture2D* LoadTexture(const std::string& name);

	bool LoadEmbeddedTexels(RawImage& rawImage, uint index);
	bool LoadExternalTexels(RawImage& rawImage, const std::string& name);

private:
	ref_ptr<Mesh> surface_;
	GameObjectAsset asset_;

	std::string path_;
	ref_ptr<GameObject> root_;

	ref_ptr<Skeleton> skeleton_;
	const aiScene* scene_;

	std::map<std::string, ref_ptr<Texture2D>> rawImages_;

	std::function<void(GameObject*, const std::string&)> callback_;
};

class GameObjectImporter : public ThreadPool {
public:
	GameObjectImporter() : ThreadPool(std::thread::hardware_concurrency()) {}
	~GameObjectImporter() {}

public:
	ref_ptr<GameObject> Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback);

protected:
	virtual void OnSchedule(Task* task);
};
