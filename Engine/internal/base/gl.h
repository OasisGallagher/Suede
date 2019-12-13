#pragma once
#include "_gl.h"
#include "mesh.h"

class GL : public _GL {
public:
	static void DrawElementsBaseVertex(MeshTopology topology, const TriangleBias& bias);
	static void DrawElementsInstancedBaseVertex(MeshTopology topology, const TriangleBias& bias, uint instance);
};
