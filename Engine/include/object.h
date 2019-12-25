#pragma once
#include "tools/enum.h"

#include <string>

#include "defines.h"

#include "tools/typeid.h"
#include "tools/string.h"
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
	Rigidbody,
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

#define SUEDE_DEFINE_METATABLE_NAME(_Name)	\
public: \
	virtual const char* metatableName() const { \
		return TypeID<ref_ptr<_Name>>::string(); \
	} \
private:

class SUEDE_API Object : public intrusive_ref_counter, public PimplIdiom {
	SUEDE_DEFINE_METATABLE_NAME(Object)
	SUEDE_DECLARE_IMPLEMENTATION(Object)

public:
	std::string GetName() const;
	void SetName(const std::string& value);

	ref_ptr<Object> Clone();
	ObjectType GetObjectType();
	uint GetInstanceID();

protected:
	Object(void* d);
};
