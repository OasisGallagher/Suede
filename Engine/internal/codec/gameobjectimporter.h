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

class GameObjectLoader : public Task, private NonCopyable {
public:
	GameObjectLoader(GameObject* root, const std::string& path, std::function<void(GameObject*)> callback);
	~GameObjectLoader();

public:
	void Apply();

private:
	virtual void Run();

private:
	struct GeometryAttribute {
		MeshTopology topology;

		std::vector<Vector3> vertices;
		std::vector<Vector3> normals;
		std::vector<Vector2> texCoords[Geometry::TexCoordsCount];
		std::vector<Vector3> tangents;
		std::vector<BlendAttribute> blendAttrs;
		std::vector<uint> indexes;
	};

	bool Initialize(Assimp::Importer& importer);

	void LoadNodeTo(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes);
	void LoadChildren(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes);
	void LoadComponents(GameObject* go, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes);

	void LoadHierarchy(GameObject* parent, aiNode* node, std::vector<ref_ptr<Material>>& materials, Geometry* geometry, SubMesh** subMeshes);

	void ReserveMemory(GeometryAttribute& attribute);
	void LoadGeometryAttribute(GeometryAttribute& attribute, SubMesh** subMeshes);
	void LoadAttributeAt(int index, GeometryAttribute& attribute, SubMesh** subMeshes);

	void LoadBoneAttribute(int meshIndex, GeometryAttribute& attribute, SubMesh** subMeshes);
	void LoadVertexAttribute(int meshIndex, GeometryAttribute& attribute);

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
	std::map<GameObject*, std::vector<ref_ptr<Component>>> componentsMap_;

	bool loaded_ = false;

	std::string path_;
	GameObject* root_;

	ref_ptr<Skeleton> skeleton_;
	const aiScene* aiScene_;

	std::map<std::string, ref_ptr<Texture2D>> rawImages_;

	std::function<void(GameObject*)> callback_;
};

class GameObjectImporter : public ScheduledThreadPool {
public:
	GameObjectImporter() : ScheduledThreadPool(std::thread::hardware_concurrency()) {}
	~GameObjectImporter() {}

public:
	ref_ptr<GameObject> Import(const std::string& path, std::function<void(GameObject*)> callback);

protected:
	virtual void OnSchedule(Task* task);
};
