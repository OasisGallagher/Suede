#pragma once

enum class ContextLimitsType {
	MaxColorAttachments,
	MaxUniformBlockSize,
	MaxUniformBufferBindings,

	MaxVertexAttribs,

	// https://www.khronos.org/opengl/wiki/Common_Mistakes#Texture_Unit
	// number of texture units was introduced as well which you can get with
	// you should not use the above because it will give a low number on modern GPUs.
	// then came the programmable GPU. There aren't texture units anymore. 
	// today, you have texture image units (TIU) which you can get with:
	// GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
	MaxTextureUnits,

	// limit of the number of textures that can be bound at any one time.
	// and this is the limit on the image unit to be passed to functions like glActiveTexture and glBindSampler.
	MaxCombinedTextureImageUnits,

	RedBits,
	GreenBits,
	BlueBits,
	AlphaBits,
	DepthBits,
	StencilBits,

	MaxClipPlanes,
	MaxTextureSize,
	MaxTextureBufferSize,

	_Count
};

class ContextLimits {
public:
	static int Get(ContextLimitsType type);
};
