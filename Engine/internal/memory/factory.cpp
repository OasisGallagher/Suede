#include "factory.h"

#include "font.h"
#include "shader.h"
#include "texture.h"
#include "material.h"

#include "world.h"
#include "gameobject.h"

#include "mesh.h"
#include "light.h"
#include "camera.h"
#include "renderer.h"
#include "rigidbody.h"
#include "transform.h"
#include "projector.h"
#include "animation.h"
#include "particlesystem.h"

#include <memory>
#include "enginedefines.h"

Factory Factory::instance;

#define FACTORY_METHOD(T)	([]() -> Object* { return new T(); })

#define ADD_FACTROY_METHOD(T) \
	AddFactoryMethod(#T, FACTORY_METHOD(T));  \
	AddFactoryMethod(+ObjectType:: ## T, FACTORY_METHOD(T))

#define ADD_COMPONENT_FACTROY_METHOD(T) \
	AddFactoryMethod(#T, FACTORY_METHOD(T)); \
	AddFactoryMethod(T::GetComponentGUID(), FACTORY_METHOD(T))

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
	ADD_COMPONENT_FACTROY_METHOD(Rigidbody);
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
