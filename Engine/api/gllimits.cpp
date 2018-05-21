#include "api/gl.h"
#include "gllimits.h"

static int limits[GLLimitsCount];
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
		}

		limits[i] = value;
	}
}

int GLLimits::Get(uint type) {
	if (type >= GLLimitsCount) {
		Debug::LogError("invalid limit type %d.", type);
		return 0;
	}

	if (limits[type] == 0) {
		Debug::LogError("invalid limit value for %d.", type);
		return 0;
	}

	return limits[type];
}
