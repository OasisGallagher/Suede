#include "contextlimits.h"

#include "gl.h"
#include "debug/debug.h"

static int limits[(int)ContextLimitsType::_Count];
#define GL_INTEGER(lim, name)	if (int value = 1) { GL::GetIntegerv(name, &value); limits[(int)lim] = value; } else (void)0

int ContextLimits::Get(ContextLimitsType type) {
	static bool initialized = false;
	if (!initialized) {
		GL_INTEGER(ContextLimitsType::MaxColorAttachments, GL_MAX_COLOR_ATTACHMENTS);
		GL_INTEGER(ContextLimitsType::MaxTextureUnits, GL_MAX_TEXTURE_UNITS);
		GL_INTEGER(ContextLimitsType::MaxCombinedTextureImageUnits, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		GL_INTEGER(ContextLimitsType::MaxUniformBufferBindings, GL_MAX_UNIFORM_BUFFER_BINDINGS);
		GL_INTEGER(ContextLimitsType::MaxVertexAttribs, GL_MAX_VERTEX_ATTRIBS);
		GL_INTEGER(ContextLimitsType::MaxUniformBlockSize, GL_MAX_UNIFORM_BLOCK_SIZE);
		GL_INTEGER(ContextLimitsType::RedBits, GL_RED_BITS);
		GL_INTEGER(ContextLimitsType::GreenBits, GL_GREEN_BITS);
		GL_INTEGER(ContextLimitsType::BlueBits, GL_BLUE_BITS);
		GL_INTEGER(ContextLimitsType::AlphaBits, GL_ALPHA_BITS);
		GL_INTEGER(ContextLimitsType::DepthBits, GL_DEPTH_BITS);
		GL_INTEGER(ContextLimitsType::StencilBits, GL_STENCIL_BITS);
		GL_INTEGER(ContextLimitsType::MaxClipPlanes, GL_MAX_CLIP_PLANES);
		GL_INTEGER(ContextLimitsType::MaxTextureSize, GL_MAX_TEXTURE_SIZE);
		GL_INTEGER(ContextLimitsType::MaxTextureBufferSize, GL_MAX_TEXTURE_BUFFER_SIZE);

		initialized = true;
	}
	SUEDE_VERIFY_INDEX((int)type, (int)ContextLimitsType::_Count, 0);
	return limits[(int)type];
}
