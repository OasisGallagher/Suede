#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <OpenThreads/Thread>

#include "mesh.h"
#include "image.h"
#include "entity.h"
#include "texture.h"
#include "renderer.h"
#include "material.h"

class LoaderCallback {
public:
	virtual void operator()() = 0;
};

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

class Loader : public OpenThreads::Thread {
public:
	Loader() : callback_(nullptr) {}
	~Loader() { if (isRunning()) { cancel(); } }

public:
	enum {
		Ok,
		Failed,
		Running,
	};

public:
	int Start();
	int GetStatus() const { return status_; }
	void SetCallback(LoaderCallback* value) { callback_ = value; }

public:
	virtual void Run() = 0;

private:
	virtual void run();

protected:
	virtual void Clear() { status_ = Failed; }
	void SetStatus(int value) { status_ = value; }

private:
	int status_;
	LoaderCallback* callback_;
};

class EntityAssetLoader : public Loader {
public:
	~EntityAssetLoader() { Clear(); }

public:
	virtual void Run();

public:
	Mesh GetSurface() { return surface_; }

	EntityAsset& GetEntityAsset() { return asset_; }
	std::vector<Renderer>& GetRenderers() { return renderers_; }

	void SetTarget(const std::string& path, Entity entity);

protected:
	virtual void Clear();

private:
	bool LoadAsset();
	bool Initialize(Assimp::Importer& importer);

	Entity LoadHierarchy(Entity parent, aiNode* node, Mesh& surface, SubMesh* subMeshes);

	void LoadNodeTo(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);
	void LoadComponents(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);
	void LoadChildren(Entity entity, aiNode* node, Mesh& surface, SubMesh* subMeshes);

	void ReserveMemory(MeshAsset& meshAsset);
	bool LoadAttribute(MeshAsset& meshAsset, SubMesh* subMeshes);
	bool LoadAttributeAt(int index, MeshAsset& meshAsset, SubMesh* subMeshes);

	void LoadVertexAttribute(int meshIndex, MeshAsset& meshAsset);
	void LoadBoneAttribute(int meshIndex, MeshAsset& meshAsset, SubMesh* subMeshes);

	void LoadMaterials();
	void LoadMaterialAsset(MaterialAsset& materialAsset, aiMaterial* material);

	bool LoadAnimation(Animation& animation);
	void LoadAnimationClip(const aiAnimation* anim, AnimationClip clip);
	void LoadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim * FindChannel(const aiAnimation* anim, const char* name);

	TexelMap* LoadTexels(const std::string& name);

	bool LoadEmbeddedTexels(TexelMap& texelMap, uint index);
	bool LoadExternalTexels(TexelMap& texelMap, const std::string& name);

	glm::vec3& AIVector3ToGLM(glm::vec3& answer, const aiVector3D& vec);
	glm::mat4& AIMaterixToGLM(glm::mat4& answer, const aiMatrix4x4& mat);
	glm::quat& AIQuaternionToGLM(glm::quat& answer, const aiQuaternion& quaternion);
	void DecomposeAIMatrix(glm::vec3& translation, glm::quat& rotation, glm::vec3& scale, const aiMatrix4x4& mat);

private:
	Entity root_;
	Mesh surface_;
	EntityAsset asset_;

	std::string path_;
	Skeleton skeleton_;
	Animation animation_;
	const aiScene* scene_;

	std::vector<Renderer> renderers_;

	typedef std::map<std::string, TexelMap*> TexelMapContainer;
	TexelMapContainer texelMapContainer_;
};
