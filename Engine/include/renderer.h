#pragma once
#include "material.h"
#include "animation.h"

class SUEDE_API Renderer : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Renderer)
	SUEDE_DECLARE_IMPLEMENTATION(Renderer)

public:
	void AddSharedMaterial(Material* material);

	/**
	 * Returns the instantiated Material assigned to the renderer.
     * Modifying material will change the material for this object only.
     * If the material is used by any other renderers, this will clone the shared material and start using it from now on.
	 */
	Material* GetMaterial(uint index);

	/**
	 * Returns the shared material of this object.
     * Modifying shared material will change the appearance of all objects using this material.
	 */
	Material* GetSharedMaterial(uint index);

	bool IsMaterialInstantiated(uint index);
	void SetMaterial(uint index, Material* value);
	void RemoveMaterialAt(uint index);
	uint GetMaterialCount();

	const Bounds& GetBounds();
	
	/**
	 * Update renderer properties to materials.
	 */
	void UpdateMaterialProperties();

protected:
	Renderer(void* d);
};

class SUEDE_API MeshRenderer : public Renderer {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(MeshRenderer)
	SUEDE_DECLARE_IMPLEMENTATION(MeshRenderer)

public:
	MeshRenderer();
};

class SUEDE_API ParticleRenderer : public Renderer {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(ParticleRenderer)
	SUEDE_DECLARE_IMPLEMENTATION(ParticleRenderer)

public:
	ParticleRenderer();
};

class SUEDE_API SkinnedMeshRenderer : public Renderer {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(SkinnedMeshRenderer)
	SUEDE_DECLARE_IMPLEMENTATION(SkinnedMeshRenderer)

public:
	SkinnedMeshRenderer();

public:
	void SetSkeleton(Skeleton* value);
};
