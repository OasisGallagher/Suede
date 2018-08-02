#pragma once
#include "gl.h"
#include "mesh.h"

enum GLLimits {
	GLLimitsMaxColorAttachments,
	GLLimitsMaxUniformBlockSize,
	GLLimitsMaxUniformBufferBindings,

	// https://www.khronos.org/opengl/wiki/Common_Mistakes#Texture_Unit
	// number of texture units was introduced as well which you can get with
	// you should not use the above because it will give a low number on modern GPUs.
	// then came the programmable GPU. There aren't texture units anymore. 
	// today, you have texture image units (TIU) which you can get with:
	// GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
	GLLimitsMaxTextureUnits,

	// limit of the number of textures that can be bound at any one time.
	// and this is the limit on the image unit to be passed to functions like glActiveTexture and glBindSampler.
	GLLimitsMaxCombinedTextureImageUnits,

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
