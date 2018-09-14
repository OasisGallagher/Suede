#pragma once
#include <vector>

#include "mesh.h"
#include "material.h"
#include "animation.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);

class SUEDE_API IRenderer : virtual public IComponent {
	RTTI_CLASS_DECLARATION(IRenderer, IComponent)

public:
	typedef SuedeEnumerable<std::vector<Material>::iterator> Enumerable;

public:
	virtual void AddMaterial(Material material) = 0;
	virtual Material GetMaterial(uint index) = 0;
	virtual Enumerable GetMaterials() = 0;
	virtual void SetMaterial(uint index, Material value) = 0;
	virtual void RemoveMaterial(Material material) = 0;
	virtual void RemoveMaterialAt(uint index) = 0;
	virtual uint GetMaterialCount() = 0;
	
	/**
	 * @brief update renderer properties to materials.
	 */
	virtual void UpdateMaterialProperties() = 0;
};

class SUEDE_API IMeshRenderer : virtual public IRenderer {
	RTTI_CLASS_DECLARATION(IMeshRenderer, IRenderer)
};

class SUEDE_API IParticleRenderer : virtual public IRenderer {
	RTTI_CLASS_DECLARATION(IParticleRenderer, IRenderer)
};

class SUEDE_API ISkinnedMeshRenderer : virtual public IRenderer {
	RTTI_CLASS_DECLARATION(ISkinnedMeshRenderer, IRenderer)

public:
	virtual void SetSkeleton(Skeleton value) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Renderer);
SUEDE_DEFINE_OBJECT_POINTER(MeshRenderer);
SUEDE_DEFINE_OBJECT_POINTER(ParticleRenderer);
SUEDE_DEFINE_OBJECT_POINTER(SkinnedMeshRenderer);
