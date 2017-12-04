#pragma once

#include "debug.h"
#include "math2.h"
#include "time2.h"
#include "screen.h"
#include "graphics.h"
#include "variables.h"
#include "resources.h"

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

class SUEDE_API Engine {
public:
	static bool Initialize();
	static void Release();

	static void Update();
	static void Resize(int w, int h);

	static World GetWorld();
};

#define SUEDE_CREATER_SPECIFICATION(T)	template<> inline T New() { \
	return dsp_cast<T>(Engine::GetWorld()->Create(ObjectType ## T)); \
}

template <class T>
inline T New() {
	throw std::string("not specification found for ") + typeid(T).name();
}

SUEDE_CREATER_SPECIFICATION(Font)
SUEDE_CREATER_SPECIFICATION(Mesh)
SUEDE_CREATER_SPECIFICATION(Shader)
SUEDE_CREATER_SPECIFICATION(SubMesh)
SUEDE_CREATER_SPECIFICATION(TextMesh)
SUEDE_CREATER_SPECIFICATION(Material)
SUEDE_CREATER_SPECIFICATION(Texture2D)
SUEDE_CREATER_SPECIFICATION(TextureCube)
SUEDE_CREATER_SPECIFICATION(RenderTexture)
SUEDE_CREATER_SPECIFICATION(MeshRenderer)
SUEDE_CREATER_SPECIFICATION(ParticleRenderer)
SUEDE_CREATER_SPECIFICATION(SkinnedMeshRenderer)

SUEDE_CREATER_SPECIFICATION(Skeleton)
SUEDE_CREATER_SPECIFICATION(Animation)
SUEDE_CREATER_SPECIFICATION(AnimationClip)
SUEDE_CREATER_SPECIFICATION(AnimationKeys)
SUEDE_CREATER_SPECIFICATION(AnimationCurve)
SUEDE_CREATER_SPECIFICATION(AnimationState)
SUEDE_CREATER_SPECIFICATION(AnimationFrame)

SUEDE_CREATER_SPECIFICATION(ParticleAnimator)
SUEDE_CREATER_SPECIFICATION(SphereParticleEmitter)

SUEDE_CREATER_SPECIFICATION(Sprite)
SUEDE_CREATER_SPECIFICATION(Camera)
SUEDE_CREATER_SPECIFICATION(Skybox)
SUEDE_CREATER_SPECIFICATION(SpotLight)
SUEDE_CREATER_SPECIFICATION(PointLight)
SUEDE_CREATER_SPECIFICATION(DirectionalLight)
SUEDE_CREATER_SPECIFICATION(ParticleSystem)

