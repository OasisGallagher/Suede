#include "gl.h"

static GLenum TopologyToGLEnum(MeshTopology topology) {
	switch (topology) {
		case MeshTopology::Points: return GL_POINTS;
		case MeshTopology::Lines: return GL_LINES;
		case MeshTopology::LineStripe: return GL_LINE_STRIP;
		case MeshTopology::Triangles: return GL_TRIANGLES;
		case MeshTopology::TriangleStripe: return GL_TRIANGLE_STRIP;
		case MeshTopology::TriangleFan: return GL_TRIANGLE_FAN;
	}

	Debug::LogError("unsupported mesh topology  %d.", topology);
	return 0;
}

void GL::DrawElementsBaseVertex(MeshTopology topology, const TriangleBias& bias) {
	_GL::DrawElementsBaseVertex(TopologyToGLEnum(topology), bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
}

void GL::DrawElementsInstancedBaseVertex(MeshTopology topology, const TriangleBias & bias, uint instance) {
	_GL::DrawElementsInstancedBaseVertex(TopologyToGLEnum(topology), bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), instance, bias.baseVertex);
}
