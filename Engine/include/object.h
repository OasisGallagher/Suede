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

	Mesh,
	TextMesh,

	Skeleton,
	AnimationClip,
	AnimationKeys,
	AnimationCurve,
	AnimationState,
	AnimationFrame,

	ParticleAnimator,
	SphereParticleEmitter,

	Entity,

	// Components
	Transform,
	ParticleSystem,
	Animation,
	MeshFilter,
	MeshRenderer,
	ParticleRenderer,
	SkinnedMeshRenderer,
	Camera,
	Projector,
	SpotLight,
	PointLight,
	DirectionalLight
)

#define SUEDE_ALL_LIGHTS	ObjectType::size()

SUEDE_DEFINE_OBJECT_POINTER(Object);

class SUEDE_API IObject {
public:
	virtual ~IObject() {}

public:
	virtual Object Clone() = 0;
	virtual ObjectType GetObjectType() = 0;
	virtual uint GetInstanceID() = 0;
};
