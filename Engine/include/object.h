#pragma once
#include <string>
#include "enginedefines.h"

enum ObjectType {
	ObjectTypeFont,
	ObjectTypeMesh,
	ObjectTypeWorld,
	ObjectTypeShader,
	ObjectTypeSubMesh,
	ObjectTypeTextMesh,
	ObjectTypeMaterial,
	ObjectTypeTransform,
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

	// Entities.
	ObjectTypeEntity,
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
