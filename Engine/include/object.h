#pragma once
#include "tools/enum.h"

#include <string>

#include "tools/typeid.h"
#include "tools/string.h"
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

SUEDE_DEFINE_OBJECT_POINTER(Object)

#define SUEDE_DEFINE_METATABLE_NAME(_Name)	\
public: \
	virtual const char* metatableName() const { \
		return TypeID<std::shared_ptr<I ## _Name>>::string(); \
	} \
private:

class SUEDE_API IObject : public std::enable_shared_from_this<IObject>, public PimplIdiom {
	SUEDE_DEFINE_METATABLE_NAME(Object)
	SUEDE_DECLARE_IMPLEMENTATION(Object)

public:
	std::string GetName() const;
	void SetName(const std::string& value);

	Object Clone();
	ObjectType GetObjectType();
	uint GetInstanceID();

protected:
	template <class T>
	std::shared_ptr<T> _shared_this_impl(T*);

protected:
	IObject(void* d);
};
