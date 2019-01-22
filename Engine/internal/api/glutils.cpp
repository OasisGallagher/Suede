#include "glutils.h"
#include "../api/gl.h"

static int limits_[GLLimitsCount];

static void InitializeGLModes();
static void InitializeGLLimits();

int GLUtils::GetLimits(GLLimits type) {
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

static GLenum TopologyToGLEnum(MeshTopology topology) {
	switch (topology) {
		case MeshTopology::Points: return GL_POINTS;
		case MeshTopology::Lines: return GL_LINES;
		case MeshTopology::LineStripe: return GL_LINE_STRIP;
		case MeshTopology::Triangles: return GL_TRIANGLES;
		case MeshTopology::TriangleStripe: return GL_TRIANGLE_STRIP;
		case MeshTopology::TriangleFan: return GL_TRIANGLE_FAN;
	}

	Debug::LogError("unsupported mesh topology  %d.", topology);
	return 0;
}

static void GLModeSet(GLMode mode, int value) {
	switch (mode) {
		case GLModePackAlignment:
			GL::PixelStorei(GL_PACK_ALIGNMENT, value);
			break;
		case GLModeUnpackAlignment:
			GL::PixelStorei(GL_UNPACK_ALIGNMENT, value);
			break;
		default:
			Debug::LogError("unsupported gl mode %d.", mode);
			break;
	}
}

static int oldModes_[GLModeCount];
static bool modeDirty_[GLModeCount];

void GLUtils::Initialize() {
	InitializeGLModes();
	InitializeGLLimits();
}

void GLUtils::DrawElementsBaseVertex(MeshTopology topology, const TriangleBias& bias) {
	GL::DrawElementsBaseVertex(TopologyToGLEnum(topology), bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
}

void GLUtils::DrawElementsInstancedBaseVertex(MeshTopology topology, const TriangleBias & bias, uint instance) {
	GL::DrawElementsInstancedBaseVertex(TopologyToGLEnum(topology), bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), instance, bias.baseVertex);
}

int GLUtils::GetGLMode(GLMode mode) {
	GLenum key = 0;
	switch (mode) {
		case GLModePackAlignment:
			key = GL_PACK_ALIGNMENT;
			break;
		case GLModeUnpackAlignment:
			key = GL_UNPACK_ALIGNMENT;
			break;
	}

	if (key == 0) {
		Debug::LogError("unsupported gl mode %d.", mode);
		return 0;
	}

	GLint value = 0;
	GL::GetIntegerv(key, &value);
	return value;
}

void GLUtils::PushGLMode(GLMode key, int value) {
	if (modeDirty_[key]) {
		Debug::LogError("mode stack overflow.");
		return;
	}

	if (oldModes_[key] != value) {
		oldModes_[key] = GetGLMode(key);

		GLModeSet(key, value);
		modeDirty_[key] = true;
	}
}

void GLUtils::PopGLMode(GLMode key) {
	if (modeDirty_[key]) {
		GLModeSet(key, oldModes_[key]);
		modeDirty_[key] = false;
	}
}

void InitializeGLModes() {
	for (int i = 0; i < GLModeCount; ++i) {
		oldModes_[i] = GLUtils::GetGLMode((GLMode)i);
	}
}

void InitializeGLLimits() {
	for (int i = 0; i < GLLimitsCount; ++i) {
		GLint value = 0;
		switch (i) {
			case GLLimitsMaxColorAttachments:
				GL::GetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
				break;
			case GLLimitsMaxTextureUnits:
				GL::GetIntegerv(GL_MAX_TEXTURE_UNITS, &value);
				break;
			case GLLimitsMaxCombinedTextureImageUnits:
				GL::GetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
				break;
			case GLLimitsMaxUniformBufferBindings:
				GL::GetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
				break;
			case GLLimitsMaxVertexAttribs:
				GL::GetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
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
				GL::GetIntegerv(GL_MAX_CLIP_PLANES, &value);
				break;
			case GLLimitsMaxTextureSize:
				GL::GetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
				break;
			case GLLimitsMaxTextureBufferSize:
				GL::GetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &value);
				break;
		}

		limits_[i] = value;
	}
}