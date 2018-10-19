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

class SUEDE_API IObject : public std::enable_shared_from_this<IObject> {
	SUEDE_DECLARE_IMPLEMENTATION(Object)

public:
	virtual ~IObject();

public:
	Object Clone();
	ObjectType GetObjectType();
	uint GetInstanceID();

public: // internal ptr helpers
	template <class T>
	T* _rptr_impl(T*) const;
	bool _d_equals_impl(void* d);

protected:
	template <class T>
	typename T::Internal* _dptr_impl(T*) const;

	template <class T>
	std::shared_ptr<T> _shared_this_impl(T*);

protected:
	IObject(void* d);

private:
	void* d_;
};
