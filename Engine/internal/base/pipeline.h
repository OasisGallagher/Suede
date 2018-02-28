#pragma once
#include "mesh.h"
#include "material.h"
#include "wrappers/gl.h"

class FramebufferBase;

struct Renderable {
	uint pass;
	uint instance;
	SubMesh subMesh;
	Material material;
	FramebufferBase* framebuffer;
};

class Pipeline {
public:
	static void Update();
	static Renderable* CreateRenderable();

private:
	static void ResetRenderable(Renderable* renderable);
	static GLenum TopologyToGLEnum(MeshTopology topology);
};

