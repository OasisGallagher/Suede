#pragma once
#include "material.h"
#include "animation.h"

class SUEDE_API Renderer : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Renderer)
	SUEDE_DECLARE_IMPLEMENTATION(Renderer)

public:
	void AddMaterial(Material* material);
	Material* GetMaterial(uint index);
	void SetMaterial(uint index, Material* value);
	void RemoveMaterial(Material* material);
	void RemoveMaterialAt(uint index);
	uint GetMaterialCount();

	const Bounds& GetBounds();
	
	/**
	 * @brief update renderer properties to materials.
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
