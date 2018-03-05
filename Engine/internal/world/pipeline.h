#pragma once
#include "mesh.h"
#include "texture.h"
#include "material.h"
#include "wrappers/gl.h"

class FramebufferBase;
struct RenderTarget {
	enum { MaxRenderTextures = 3 };
	FramebufferBase* framebuffer;

	RenderTexture depthTexture;
	RenderTexture renderTextures[MaxRenderTextures];
};

struct Renderable {
	uint pass;
	uint instance;
	SubMesh subMesh;
	Material material;

	RenderTarget target;
	FramebufferBase* framebuffer;
};

class Pipeline {
public:
	static void Update();
	static Renderable* CreateRenderable();

private:
	static void ResetState();
	static void SortRenderables();
	static void Render(Renderable& p);
	static void ResetRenderable(Renderable* renderable);
	static GLenum TopologyToGLEnum(MeshTopology topology);
};

