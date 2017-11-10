#pragma once
#include <memory>
#include <string>
#include "defines.h"

enum ObjectType {
	ObjectTypeMesh,
	ObjectTypeWorld,
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

typedef std::shared_ptr<class IObject> Object;

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
template<class T, class U>
inline T dsp_cast(const std::shared_ptr<U>& ptr) {
	typename T::element_type* p = dynamic_cast<typename T::element_type*>(ptr.get());
	if (p != nullptr) {
		return T(ptr, p);
	}

	return T();
}
