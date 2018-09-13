#include "internal/memory/factory.h"

#include "internal/base/fontinternal.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/materialinternal.h"

#include "internal/components/animationinternal.h"
#include "internal/components/particlesysteminternal.h"

#include "internal/entities/entityinternal.h"

#include "internal/world/worldinternal.h"

Factory Factory::instance;

#define ADD_FACTROY_METHOD(name) \
	AddFactoryMethod(#name, name ## Internal::Create);  \
	AddFactoryMethod(ObjectType:: ## name, name ## Internal::Create)

Factory::Factory() {
	std::fill(methodArray_, methodArray_ + ObjectType::size(), nullptr);

	//ADD_FACTROY_METHOD(World);

	ADD_FACTROY_METHOD(Font);
	ADD_FACTROY_METHOD(Shader);
	ADD_FACTROY_METHOD(Entity);
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
}
