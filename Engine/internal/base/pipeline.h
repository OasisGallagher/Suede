#pragma once
#include "mesh.h"
#include "material.h"

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
};
