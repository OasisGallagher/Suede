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

Factory Factory::instance;

#define FACTORY_METHOD(T)	([]() -> Object* { return new T(); })

#define ADD_FACTROY_METHOD(T) \
	AddFactoryMethod(#T, FACTORY_METHOD(T));  \
	AddFactoryMethod(+ObjectType:: ## T, FACTORY_METHOD(T))

#define ADD_COMPONENT_FACTROY_METHOD(T) \
	AddFactoryMethod(#T, FACTORY_METHOD(T)); \
	AddFactoryMethod(T::GetComponentGUID(), FACTORY_METHOD(T))

Factory::Factory() {
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
