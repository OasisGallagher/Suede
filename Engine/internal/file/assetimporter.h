#pragma once

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
	Sprite Import(const std::string& path);
	bool ImportTo(Sprite sprite, const std::string& path);

public:
	Animation GetAnimation() { return animation_; }

private:
	void Clear();
	void Initialize(const std::string& path, Assimp::Importer &importer);
	void CombineAttribute(MeshAttribute& dest, const MeshAttribute& src);

	Sprite ReadHierarchy(Sprite parent, aiNode* node, MeshAttribute* attributes, Material* materials);

	void ReadNodeTo(Sprite sprite, aiNode* node, MeshAttribute* attribute, Material* materials);
	void ReadComponents(Sprite sprite, aiNode* node, MeshAttribute* attributes, Material* materials);

	void ReadChildren(Sprite sprite, aiNode* node, MeshAttribute* attribute, Material* materials);

	bool ReadAttributes(MeshAttribute* attribute);
	bool ReadAttribute(MeshAttribute& attribute, int index);
	void ReadVertexAttributes(int index, MeshAttribute& attribute);
	void ReadBoneAttributes(int index, MeshAttribute& attribute);

	struct MaterialAttribute {
		MaterialAttribute() : twoSided(false), gloss(0), mainColor(1) {}

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
	bool ReadMaterial(Material material, const MaterialAttribute& attribute);
	void ReadMaterialAttribute(MaterialAttribute& attribute, aiMaterial* material);

	bool ReadAnimation(Animation& animation);
	void ReadAnimationClip(const aiAnimation* anim, AnimationClip clip);
	void ReadAnimationNode(const aiAnimation* anim, const aiNode* paiNode, SkeletonNode* pskNode);
	const aiNodeAnim * FindChannel(const aiAnimation* anim, const char* name);

	Texture CreateTexture(const std::string& name);
	Texture GetDefaultMainTexture();

private:
	Skeleton skeleton_;
	std::string path_;
	const aiScene* scene_;

	Animation animation_;
};
