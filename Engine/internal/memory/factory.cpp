#include "factory.h"

#include "internal/base/fontinternal.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/materialinternal.h"

#include "internal/world/worldinternal.h"
#include "internal/gameobject/gameobjectinternal.h"

#include "internal/components/meshinternal.h"
#include "internal/components/lightinternal.h"
#include "internal/components/camerainternal.h"
#include "internal/components/rendererinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/components/projectorinternal.h"
#include "internal/components/animationinternal.h"
#include "internal/components/particlesysteminternal.h"

Factory Factory::instance;

#define ADD_FACTROY_METHOD(T) \
	AddFactoryMethod(#T, FactoryCreate<T>);  \
	AddFactoryMethod(+ObjectType:: ## T, FactoryCreate<T>)

#define ADD_COMPONENT_FACTROY_METHOD(T) \
	AddFactoryMethod(T::element_type::GetComponentGUID(), FactoryCreate<T>)

Factory::Factory() {
	std::fill(methodArray_, methodArray_ + ObjectType::size(), nullptr);

	ADD_FACTROY_METHOD(GameObject);

	ADD_FACTROY_METHOD(Font);
	ADD_FACTROY_METHOD(Mesh);
	ADD_FACTROY_METHOD(SubMesh);
	ADD_FACTROY_METHOD(Shader);
	ADD_FACTROY_METHOD(Material);
	ADD_FACTROY_METHOD(Texture2D);
	ADD_FACTROY_METHOD(TextureCube);
	ADD_FACTROY_METHOD(RenderTexture);
	ADD_FACTROY_METHOD(MRTRenderTexture);
	ADD_FACTROY_METHOD(TextureBuffer);
	
	ADD_FACTROY_METHOD(Skeleton);
	ADD_FACTROY_METHOD(AnimationClip);
	ADD_FACTROY_METHOD(AnimationKeys);
	ADD_FACTROY_METHOD(AnimationCurve);
	ADD_FACTROY_METHOD(AnimationState);
	ADD_FACTROY_METHOD(AnimationFrame);

	ADD_FACTROY_METHOD(ParticleAnimator);
	ADD_FACTROY_METHOD(SphereParticleEmitter);

	ADD_COMPONENT_FACTROY_METHOD(Camera);
	ADD_COMPONENT_FACTROY_METHOD(Transform);
	ADD_COMPONENT_FACTROY_METHOD(Animation);
	ADD_COMPONENT_FACTROY_METHOD(Projector);
	ADD_COMPONENT_FACTROY_METHOD(TextMesh);
	ADD_COMPONENT_FACTROY_METHOD(MeshFilter);
	ADD_COMPONENT_FACTROY_METHOD(ParticleSystem);

	ADD_COMPONENT_FACTROY_METHOD(MeshRenderer);
	ADD_COMPONENT_FACTROY_METHOD(SkinnedMeshRenderer);
	ADD_COMPONENT_FACTROY_METHOD(ParticleRenderer);

	ADD_COMPONENT_FACTROY_METHOD(Light);
}
