#pragma once
#include "tools/enum.h"

#include <string>

#include "enginedefines.h"
#include "tools/pimplidiom.h"

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

	GameObject,

	// Components
	Transform,
	Light,
	Camera,
	Projector,
	ParticleSystem,
	Animation,
	MeshFilter,
	MeshRenderer,
	ParticleRenderer,
	SkinnedMeshRenderer,

	// 
	CustomBehaviour
)

SUEDE_DEFINE_OBJECT_POINTER(Object);

class SUEDE_API IObject : public std::enable_shared_from_this<IObject>, public PimplIdiom {
	SUEDE_DECLARE_IMPLEMENTATION(Object)

public:
	Object Clone();
	ObjectType GetObjectType();
	uint GetInstanceID();

protected:
	template <class T>
	std::shared_ptr<T> _shared_this_impl(T*);

protected:
	IObject(void* d);
};
