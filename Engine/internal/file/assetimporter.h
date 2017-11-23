#include "surface.h"
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
	Surface ImportSurface(const std::string& path);

public:
	Animation GetAnimation() { return animation_; }

private:
	void Clear();
	void Initialize(const std::string& path, Assimp::Importer &importer);

	Sprite ReadHierarchy(Sprite parent, aiNode* node, Surface* surfaces, Material* materials);

	void ReadNodeTo(Sprite sprite, aiNode* node, Surface* surfaces, Material* materials);
	void ReadChildren(Sprite sprite, aiNode* node, Surface* surfaces, Material* materials);

	bool ReadSurfaces(Surface* surfaces);
	bool ReadSurface(Surface surface, int index);
	void ReadSurfaceAttributes(Surface surface, int index, SurfaceAttribute& attribute);
	void ReadVertexAttributes(int index, SurfaceAttribute& attribute);
	void ReadBoneAttributes(int index, SurfaceAttribute& attribute);

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