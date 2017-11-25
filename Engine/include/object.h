#pragma once
#include <string>
#include "defines.h"

enum ObjectType {
	ObjectTypeFont,
	ObjectTypeMesh,
	ObjectTypeWorld,
	ObjectTypeShader,
	ObjectTypeSubMesh,
	ObjectTypeTextMesh,
	ObjectTypeMaterial,
	ObjectTypeTexture2D,
	ObjectTypeTextureCube,
	ObjectTypeRenderTexture,
	ObjectTypeMeshRenderer,
	ObjectTypeParticleRenderer,
	ObjectTypeSkinnedMeshRenderer,

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

SUEDE_DEFINE_OBJECT_POINTER(Object);

class SUEDE_API IObject : public std::enable_shared_from_this<IObject> {
public:
	virtual ~IObject() {}

public:
	virtual Object Clone() = 0;
	virtual ObjectType GetType() = 0;
	virtual uint GetInstanceID() = 0;
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
