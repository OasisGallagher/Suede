#pragma once
#include "gl.h"
#include "mesh.h"

enum {
	GLLimitsMaxColorAttachments,
	GLLimitsMaxTextureUnits,
	GLLimitsMaxUniformBufferBindings,
	GLLimitsMaxUniformBlockSize,

	GLLimitsRedBits,
	GLLimitsGreenBits,
	GLLimitsBlueBits,
	GLLimitsAlphaBits,
	GLLimitsDepthBits,
	GLLimitsStencilBits,

	GLLimitsMaxClipPlanes,
	GLLimitsMaxTextureSize,
	GLLimitsMaxTextureBufferSize,

	GLLimitsCount,
};

class GLLimits {
public:
	static void Initialize();

public:
	static int Get(uint type);
};

enum GLMode {
	GLModePackAlignment,
	GLModeUnpackAlignment,

	GLModeCount,
};

class GLUtils {
public:
	static void Initialize();

public:
	static void DrawElementsBaseVertex(MeshTopology topology, const TriangleBias& bias);
	static void DrawElementsInstancedBaseVertex(MeshTopology topology, const TriangleBias& bias, uint instance);

	static int GetGLMode(GLMode key);
	static void PushGLMode(GLMode key, int value);
	static void PopGLMode(GLMode key);
};

