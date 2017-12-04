#include "font.h"
#include "mesh.h"
#include "light.h"
#include "world.h"
#include "sprite.h"
#include "camera.h"
#include "skybox.h"
#include "shader.h"
#include "texture.h"
#include "material.h"
#include "renderer.h"
#include "animation.h"
#include "particlesystem.h"

#define DEFINE_OBJECT_CREATER(T)	T New ## T() { \
	return dsp_cast<T>(WorldInstance()->Create(ObjectType ## T)); \
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
DEFINE_OBJECT_CREATER(MeshRenderer)
DEFINE_OBJECT_CREATER(ParticleRenderer)
DEFINE_OBJECT_CREATER(SkinnedMeshRenderer)

DEFINE_OBJECT_CREATER(Skeleton)
DEFINE_OBJECT_CREATER(Animation)
DEFINE_OBJECT_CREATER(AnimationClip)
DEFINE_OBJECT_CREATER(AnimationKeys)
DEFINE_OBJECT_CREATER(AnimationCurve)
DEFINE_OBJECT_CREATER(AnimationState)
DEFINE_OBJECT_CREATER(AnimationFrame)

DEFINE_OBJECT_CREATER(ParticleAnimator)
DEFINE_OBJECT_CREATER(SphereParticleEmitter)

DEFINE_OBJECT_CREATER(Sprite)
DEFINE_OBJECT_CREATER(Camera)
DEFINE_OBJECT_CREATER(Skybox)
DEFINE_OBJECT_CREATER(SpotLight)
DEFINE_OBJECT_CREATER(PointLight)
DEFINE_OBJECT_CREATER(DirectionalLight)
DEFINE_OBJECT_CREATER(ParticleSystem)
