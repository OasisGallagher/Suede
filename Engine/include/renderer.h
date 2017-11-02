#pragma once

#include "object.h"
#include "surface.h"
#include "material.h"
#include "animation.h"

enum RenderQueue {
	RenderQueueBackground = 1000,
	RenderQueueGeometry = 2000,
	RenderQueueTransparent = 4000,
};

enum RenderStateType {
	Cull,
	Blend,
	DepthTest,
	DepthWrite,
	RasterizerDiscard,

	RenderStateCount,
};

enum {
	None,
	Front,
	Back,
	On,
	Off,

	Never,
	Less,
	LessEqual,
	Equal,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,

	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DestAlpha,
	OneMinusDestAlpha,
};

typedef std::shared_ptr<class ISprite> Sprite;

class ENGINE_EXPORT IRenderer : virtual public IObject {
public:
	virtual void RenderSprite(Sprite sprite) = 0;
	virtual void RenderSurface(Surface surface) = 0;
	
	virtual void AddMaterial(Material material) = 0;
	virtual Material GetMaterial(int index) = 0;
	virtual void SetMaterial(int index, Material value) = 0;
	virtual int GetMaterialCount() = 0;

	virtual void SetRenderQueue(int value) = 0;
	virtual int GetRenderQueue() = 0;

	/**
	 * @param Cull: Front, Back, Off
	 * @param DepthTest: Never, Less, LessEqual, Equal, Greater, NotEqual, GreaterEqual, Always
	 * @param DepthWrite: On, Off
	 * @param Blend0: Off, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha
	 * @param Blend1: None, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha
	 * @param RasterizerDiscard: On, Off
	 */
	virtual void SetRenderState(RenderStateType type, int parameter0, int parameter1 = 0) = 0;
};

class ENGINE_EXPORT ISurfaceRenderer : virtual public IRenderer {

};

class ENGINE_EXPORT IParticleRenderer : virtual public IRenderer {
};

class ENGINE_EXPORT ISkinnedSurfaceRenderer : virtual public IRenderer {
public:
	virtual void SetSkeleton(Skeleton value) = 0;
};

typedef std::shared_ptr<IRenderer> Renderer;
typedef std::shared_ptr<ISurfaceRenderer> SurfaceRenderer;
typedef std::shared_ptr<IParticleRenderer> ParticleRenderer; 
typedef std::shared_ptr<ISkinnedSurfaceRenderer> SkinnedSurfaceRenderer;

