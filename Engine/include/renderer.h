#pragma once
#include <vector>

#include "mesh.h"
#include "material.h"
#include "animation.h"

SUEDE_DEFINE_OBJECT_POINTER(GameObject);

class SUEDE_API IRenderer : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DECLARE_IMPL(Renderer)

public:
	typedef SuedeEnumerable<std::vector<Material>::iterator> Enumerable;

public:
	void AddMaterial(Material material);
	Material GetMaterial(uint index);
	Enumerable GetMaterials();
	void SetMaterial(uint index, Material value);
	void RemoveMaterial(Material material);
	void RemoveMaterialAt(uint index);
	uint GetMaterialCount();
	
	/**
	 * @brief update renderer properties to materials.
	 */
	void UpdateMaterialProperties();

protected:
	IRenderer(void* d);
};

class SUEDE_API IMeshRenderer : public IRenderer {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DECLARE_IMPL(MeshRenderer)

public:
	IMeshRenderer();
};

class SUEDE_API IParticleRenderer : public IRenderer {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DECLARE_IMPL(ParticleRenderer)

public:
	IParticleRenderer();
};

class SUEDE_API ISkinnedMeshRenderer : public IRenderer {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DECLARE_IMPL(SkinnedMeshRenderer)

public:
	ISkinnedMeshRenderer();

public:
	void SetSkeleton(Skeleton value);
};

SUEDE_DEFINE_OBJECT_POINTER(Renderer);
SUEDE_DEFINE_OBJECT_POINTER(MeshRenderer);
SUEDE_DEFINE_OBJECT_POINTER(ParticleRenderer);
SUEDE_DEFINE_OBJECT_POINTER(SkinnedMeshRenderer);
