#pragma once
#include "tools/enum.h"

#include <string>
#include "enginedefines.h"

BETTER_ENUM(ObjectType, int,
	Font,
	Shader,
	SubMesh,
	Material,
	Texture2D,
	TextureCube,
	RenderTexture,
	MRTRenderTexture,
	TextureBuffer,

	Skeleton,
	Animation,
	AnimationClip,
	AnimationKeys,
	AnimationCurve,
	AnimationState,
	AnimationFrame,

	ParticleAnimator,
	SphereParticleEmitter,

	// Components
	Transform,
	ParticleSystem,
	Mesh,
	TextMesh,
	MeshRenderer,
	ParticleRenderer,
	SkinnedMeshRenderer,

	// Entities.
	Entity,
	Camera,
	Projector,
	SpotLight,
	PointLight,
	DirectionalLight
)

#define SUEDE_ALL_LIGHTS	ObjectType::count()

SUEDE_DEFINE_OBJECT_POINTER(Object);

class SUEDE_API IObject {
public:
	virtual ~IObject() {}

public:
	virtual Object Clone() = 0;
	virtual ObjectType GetType() = 0;
	virtual uint GetInstanceID() = 0;
};
