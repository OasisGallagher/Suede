#pragma once

#include "mesh.h"
#include "material.h"
#include "animation.h"

enum RenderQueue {
	RenderQueueBackground = 1000,
	RenderQueueGeometry = 2000,
	RenderQueueTransparent = 4000,
};

SUEDE_DEFINE_OBJECT_POINTER(Sprite);

class SUEDE_API IRenderer : virtual public IObject {
public:
	virtual void RenderSprite(Sprite sprite) = 0;
	virtual void RenderMesh(Mesh mesh, Material material) = 0;
	
	virtual void AddMaterial(Material material) = 0;
	virtual Material GetMaterial(int index) = 0;
	virtual void SetMaterial(int index, Material value) = 0;
	virtual int GetMaterialCount() = 0;

	virtual void SetRenderQueue(int value) = 0;
	virtual int GetRenderQueue() = 0;
};

class SUEDE_API IMeshRenderer : virtual public IRenderer {
};

class SUEDE_API IParticleRenderer : virtual public IRenderer {
};

class SUEDE_API ISkinnedMeshRenderer : virtual public IRenderer {
public:
	virtual void SetSkeleton(Skeleton value) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Renderer);
SUEDE_DEFINE_OBJECT_POINTER(MeshRenderer);
SUEDE_DEFINE_OBJECT_POINTER(ParticleRenderer);
SUEDE_DEFINE_OBJECT_POINTER(SkinnedMeshRenderer);

