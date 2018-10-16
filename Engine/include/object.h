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
	bool d_equals(void* d);
	template <class T>
	typename T::Internal* dptr_impl(T*) const { return (T::Internal*)d_; }

	template <class T>
	T* rptr_impl(T*) const { return (T*)d_; }

protected:
	template <class T>
	typename std::shared_ptr<T> SharedThisTraits(T*);

protected:
	IObject(void* d);

private:
	void* d_;
};

#define dptr()	dptr_impl(this)
#define rptr(o)	(o).get()->rptr_impl(this)	

template <class T>
typename std::shared_ptr<T> IObject::SharedThisTraits(T*) {
	return suede_dynamic_cast<std::shared_ptr<T>>(shared_from_this());
}

#define SharedThis()		SharedThisTraits(this)
