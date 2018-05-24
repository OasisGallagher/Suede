#pragma once
#include "types.h"

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

	GLLimitsCount,
};

class GLLimits {
public:
	static void Initialize();

public:
	static int Get(uint type);
};
