#include "font.h"
#include "mesh.h"
#include "light.h"
#include "world.h"
#include "camera.h"
#include "shader.h"
#include "texture.h"
#include "material.h"
#include "renderer.h"
#include "projector.h"
#include "animation.h"
#include "gameobject.h"
#include "particlesystem.h"

#define DEFINE_OBJECT_CREATER(T)	T New ## T() { \
	return suede_dynamic_cast<T>(World::instance()->CreateObject(ObjectType:: ## T)); \
}

DEFINE_OBJECT_CREATER(Font)
DEFINE_OBJECT_CREATER(Mesh)
DEFINE_OBJECT_CREATER(Shader)
DEFINE_OBJECT_CREATER(SubMesh)
DEFINE_OBJECT_CREATER(TextMesh)
DEFINE_OBJECT_CREATER(Material)
DEFINE_OBJECT_CREATER(Texture2D)
DEFINE_OBJECT_CREATER(TextureCube)
DEFINE_OBJECT_CREATER(RenderTexture)
DEFINE_OBJECT_CREATER(MRTRenderTexture)
DEFINE_OBJECT_CREATER(TextureBuffer)

DEFINE_OBJECT_CREATER(Skeleton)
DEFINE_OBJECT_CREATER(AnimationClip)
DEFINE_OBJECT_CREATER(AnimationKeys)
DEFINE_OBJECT_CREATER(AnimationCurve)
DEFINE_OBJECT_CREATER(AnimationState)
DEFINE_OBJECT_CREATER(AnimationFrame)

DEFINE_OBJECT_CREATER(ParticleAnimator)
DEFINE_OBJECT_CREATER(SphereParticleEmitter)

DEFINE_OBJECT_CREATER(GameObject)
