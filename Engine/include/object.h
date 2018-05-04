#pragma once
#include <string>
#include "enginedefines.h"

enum ObjectType {
	ObjectTypeFont,
	ObjectTypeWorld,
	ObjectTypeShader,
	ObjectTypeSubMesh,
	ObjectTypeMaterial,
	ObjectTypeTexture2D,
	ObjectTypeTextureCube,
	ObjectTypeRenderTexture,

	ObjectTypeSkeleton,
	ObjectTypeAnimation,
	ObjectTypeAnimationClip,
	ObjectTypeAnimationKeys,
	ObjectTypeAnimationCurve,
	ObjectTypeAnimationState,
	ObjectTypeAnimationFrame,

	ObjectTypeParticleAnimator,
	ObjectTypeSphereParticleEmitter,

	// Components
	ObjectTypeTransform,
	ObjectTypeParticleSystem,
	ObjectTypeMesh,
	ObjectTypeTextMesh,
	ObjectTypeMeshRenderer,
	ObjectTypeParticleRenderer,
	ObjectTypeSkinnedMeshRenderer,

	// Entities.
	ObjectTypeEntity,
	ObjectTypeCamera,
	ObjectTypeProjector,
	ObjectTypeSpotLight,
	ObjectTypePointLight,
	ObjectTypeDirectionalLight,

	ObjectTypeCount,

	ObjectTypeLights,
};

SUEDE_DEFINE_OBJECT_POINTER(Object);

class SUEDE_API IObject {
public:
	virtual ~IObject() {}

public:
	virtual Object Clone() = 0;
	virtual ObjectType GetType() = 0;
	virtual uint GetInstanceID() = 0;
};
