#pragma once

#include <map>

#include "mesh.h"
#include "renderer.h"
#include "animation.h"

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiNodeAnim;
struct aiAnimation;
namespace Assimp {
	class Importer;
}

class AssetImporter {
public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

public:
	Animation GetAnimation() { return animation_; }

private:
	void Clear();
	bool Initialize(const std::string& path, Assimp::Importer &importer);

	Entity ReadHierarchy(Entity parent, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials);

	void ReadNodeTo(Entity entity, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials);
	void ReadComponents(Entity entity, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials);
	void ReadChildren(Entity entity, aiNode* node, Mesh& surface, TriangleBase* bases, Material* materials);

	bool ReadAttribute(MeshAttribute& attribute, TriangleBase* bases);
	void ReserveMemory(MeshAttribute& attribute);
	bool ReadAttributeAt(int index, MeshAttribute& attribute, TriangleBase* bases);

	void ReadVertexAttribute(int meshIndex, MeshAttribute& attribute);
	void ReadBoneAttribute(int meshIndex, MeshAttribute& attribute, TriangleBase* bases);

	struct MaterialAttribute {
		MaterialAttribute();

		std::string name;

		bool twoSided;
		float gloss;

		glm::vec4 mainColor;
		glm::vec3 specularColor;
		glm::vec3 emissiveColor;

		Texture mainTexture;
		Texture bumpTexture;
		Texture specularTexture;
		Texture emissiveTexture;
		Texture lightmapTexture;
	};

	bool ReadMaterials(Material* materials);
	bool ReadMaterial(Material material, const MaterialAttribute& surface);
	void ReadMaterialAttribute(MaterialAttribute& surface, aiMaterial* material);

	bool ReadAnimation(Animation& animation);
	void ReadAnimationClip(const aiAnimation* anim, AnimationClip clip);
	void ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim * FindChannel(const aiAnimation* anim, const char* name);

	Texture GetTexture(const std::string& name);
	Texture ReadExternalTexture(const std::string& name);
	Texture ReadEmbeddedTexture(uint index);

private:
	Skeleton skeleton_;
	std::string path_;
	const aiScene* scene_;
	Animation animation_;

	typedef std::map<std::string, Texture> TextureContainer;
	TextureContainer textures_;
};
