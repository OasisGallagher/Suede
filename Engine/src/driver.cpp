#include "driver.h"
#include "debug/debug.h"
#include "wrappers/gl.h"

const char* Driver::GetVendor() {
	return GL::GetString(GL_VENDOR);
}

const char* Driver::GetRenderer() {
	return GL::GetString(GL_RENDERER);
}

const char* Driver::GetVersion() {
	return GL::GetString(GL_VERSION);
}

const char* Driver::GetGLSLVersion() {
	return GL::GetString(GL_SHADING_LANGUAGE_VERSION);
}

uint Driver::GetLimits(Limits limits) {
	GLenum name = 0;
	switch (limits) {
		case RedBits: name = GL_RED_BITS; break;
		case GreenBits: name = GL_GREEN_BITS; break;
		case BlueBits: name = GL_BLUE_BITS; break;
		case AlphaBits: name = GL_ALPHA_BITS; break;
		case DepthBits: name = GL_DEPTH_BITS; break;
		case StencilBits: name = GL_STENCIL_BITS; break;

		case MaxClipPlanes: name = GL_MAX_TEXTURE_SIZE; break;
		case MaxTextureSize: name = GL_MAX_CLIP_PLANES; break;
	}

	if (name == 0) {
		Debug::LogError("invalid limits name %d.", limits);
		return 0;
	}

	uint value = 0;
	GL::GetIntegerv(name, (GLint*)&value);
	return value;
}
