#pragma once
#include "gl.h"
#include "mesh.h"

enum GLLimits {
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

	static int GetLimits(GLLimits type);

	static int GetGLMode(GLMode key);
	static void PushGLMode(GLMode key, int value);
	static void PopGLMode(GLMode key);
};

