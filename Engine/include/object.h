#pragma once
#include <string>
#include "defines.h"

enum ObjectType {
	ObjectTypeMesh,
	ObjectTypeWorld,
	ObjectTypeFont,
	ObjectTypeShader,
	ObjectTypeSurface,
	ObjectTypeMaterial,
	ObjectTypeTexture2D,
	ObjectTypeTextureCube,
	ObjectTypeRenderTexture,
	ObjectTypeSurfaceRenderer,
	ObjectTypeParticleRenderer,
	ObjectTypeSkinnedSurfaceRenderer,

	ObjectTypeSkeleton,
	ObjectTypeAnimation,
	ObjectTypeAnimationClip,
	ObjectTypeAnimationKeys,
	ObjectTypeAnimationCurve,
	ObjectTypeAnimationState,
	ObjectTypeAnimationFrame,

	ObjectTypeParticleAnimator,
	ObjectTypeSphereParticleEmitter,

	// Sprites.
	ObjectTypeSprite,
	ObjectTypeCamera,
	ObjectTypeSkybox,
	ObjectTypeSpotLight,
	ObjectTypePointLight,
	ObjectTypeDirectionalLight,
	ObjectTypeParticleSystem,

	ObjectTypeCount,

	ObjectTypeLights,
};

DEFINE_OBJECT_PTR(Object);

class ENGINE_EXPORT IObject : public std::enable_shared_from_this<IObject> {
public:
	virtual ~IObject() {}

public:
	virtual Object Clone() = 0;
	virtual ObjectType GetType() = 0;
	virtual unsigned GetInstanceID() = 0;
};

/**
 * dynamic shared_ptr cast.
 */
template<class T, class Ptr>
inline T dsp_cast(const Ptr& ptr) {
	typedef typename T::element_type Element;
	Element* p = dynamic_cast<Element*>(ptr.get());
	if (p != nullptr) {
		return T(ptr, p);
	}

	return T();
}
