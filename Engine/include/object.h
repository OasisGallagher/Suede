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
	SUEDE_DECLARE_IMPL(Object)

public:
	virtual ~IObject();

public:
	Object Clone();
	ObjectType GetObjectType();
	uint GetInstanceID();

public:
	template <class T>
	T* _rptr_impl(T*) const { return (T*)d_; }
	bool _d_equals_impl(void* d) { return d_ == d; }

protected:
	template <class T>
	typename T::Internal* _dptr_impl(T*) const { return (T::Internal*)d_; }

	template <class T>
	typename std::shared_ptr<T> _shared_this_impl(T*) { return suede_dynamic_cast<std::shared_ptr<T>>(shared_from_this()); }

protected:
	IObject(void* d);

private:
	void* d_;
};

#define _dptr()			_dptr_impl(this)
#define _d_equals(o)	(o).get()->_d_equals_impl(this)
#define _rptr(o)		(o).get()->_rptr_impl(this)
#define _shared_this()	_shared_this_impl(this)
