#pragma once
#include "mesh.h"
#include "material.h"
#include "wrappers/gl.h"

struct Renderable {
	enum { MaxProperties = 3 };
	Renderable() { properties[0] = nullptr; }

	uint pass;
	uint instance;
	SubMesh subMesh;
	Material material;
	Property* properties[MaxProperties];
};

class Pipeline {
public:
	static void Update();
	static Property* CreateProperty();
	static Renderable* CreateRenderable();

private:
	static void ResetRenderable(Renderable* renderable);
	static GLenum TopologyToGLEnum(MeshTopology topology);
};

