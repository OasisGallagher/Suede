#include "api/gl.h"
#include "gllimits.h"

static int limits_[GLLimitsCount];

void GLLimits::Initialize() {
	for (int i = 0; i < GLLimitsCount; ++i) {
		GLint value = 0;
		switch (i) {
			case GLLimitsMaxColorAttachments:
				GL::GetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
				break;
			case GLLimitsMaxTextureUnits:
				GL::GetIntegerv(GL_MAX_TEXTURE_UNITS, &value);
				break;
			case GLLimitsMaxUniformBufferBindings:
				GL::GetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
				break;
			case GLLimitsMaxUniformBlockSize:
				GL::GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
				break;
			case GLLimitsRedBits:
				GL::GetIntegerv(GL_RED_BITS, &value); 
				break;
			case GLLimitsGreenBits: 
				GL::GetIntegerv(GL_GREEN_BITS, &value); 
				break;
			case GLLimitsBlueBits: 
				GL::GetIntegerv(GL_BLUE_BITS, &value); 
				break;
			case GLLimitsAlphaBits: 
				GL::GetIntegerv(GL_ALPHA_BITS, &value); 
				break;
			case GLLimitsDepthBits: 
				GL::GetIntegerv(GL_DEPTH_BITS, &value); 
				break;
			case GLLimitsStencilBits: 
				GL::GetIntegerv(GL_STENCIL_BITS, &value); 
				break;
			case GLLimitsMaxClipPlanes: 
				GL::GetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
				break;
			case GLLimitsMaxTextureSize: 
				GL::GetIntegerv(GL_MAX_CLIP_PLANES, &value); 
				break;
		}

		limits_[i] = value;
	}
}

int GLLimits::Get(uint type) {
	if (type >= GLLimitsCount) {
		Debug::LogError("invalid limit type %d.", type);
		return 0;
	}

	if (limits_[type] == 0) {
		Debug::LogError("invalid limit value for %d.", type);
		return 0;
	}

	return limits_[type];
}
