#pragma once
#include "types.h"

enum {
	GLLimitsMaxColorAttachments,
	GLLimitsMaxTextureUnits,
	GLLimitsMaxUniformBufferBindings,
	GLLimitsMaxUniformBlockSize,

	GLLimitsCount,
};

class GLLimits {
public:
	static void Initialize();

public:
	static int Get(uint type);
};
