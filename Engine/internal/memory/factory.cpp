#include "internal/memory/factory.h"

#include "internal/base/shaderinternal.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/rendererinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/animationinternal.h"

#include "internal/world/worldinternal.h"

#include "internal/sprites/lightinternal.h"
#include "internal/sprites/skyboxinternal.h"
#include "internal/sprites/camerainternal.h"
#include "internal/sprites/particlesysteminternal.h"

Factory Factory::instance;

#define ADD_FACTROY_METHOD(name) \
	AddFactoryMethod(#name, name ## Internal::Create);  \
	AddFactoryMethod(ObjectType ## name, name ## Internal::Create)

Factory::Factory() {
	std::fill(methodArray_, methodArray_ + ObjectTypeCount, nullptr);

	//ADD_FACTROY_METHOD(World);

	ADD_FACTROY_METHOD(Mesh);
	ADD_FACTROY_METHOD(Camera);
	ADD_FACTROY_METHOD(Shader);
	ADD_FACTROY_METHOD(Skybox);
	ADD_FACTROY_METHOD(Sprite);
	ADD_FACTROY_METHOD(Surface);
	ADD_FACTROY_METHOD(Material);
	ADD_FACTROY_METHOD(SurfaceRenderer);
	ADD_FACTROY_METHOD(SkinnedSurfaceRenderer);
	ADD_FACTROY_METHOD(Texture2D);
	ADD_FACTROY_METHOD(TextureCube);
	ADD_FACTROY_METHOD(RenderTexture);
	
	ADD_FACTROY_METHOD(Skeleton);
	ADD_FACTROY_METHOD(Animation);
	ADD_FACTROY_METHOD(AnimationClip);
	ADD_FACTROY_METHOD(AnimationKeys);
	ADD_FACTROY_METHOD(AnimationCurve);
	ADD_FACTROY_METHOD(AnimationState);
	ADD_FACTROY_METHOD(AnimationFrame);

	ADD_FACTROY_METHOD(ParticleSystem);
	ADD_FACTROY_METHOD(ParticleAnimator);
	ADD_FACTROY_METHOD(SphereParticleEmitter);

	ADD_FACTROY_METHOD(SpotLight);
	ADD_FACTROY_METHOD(PointLight);
	ADD_FACTROY_METHOD(DirectionalLight);
}
