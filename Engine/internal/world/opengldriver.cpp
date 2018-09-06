#include "opengldriver.h"
#include "debug/debug.h"
#include "../api/gl.h"

#ifndef _STDCALL
#define _STDCALL __stdcall
#endif

static void _STDCALL GLDebugMessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam
);

bool OpenGLDriver::Load() {
	glewExperimental = true;
	GLenum status = glewInit();
	if (status != GLEW_OK) {
		Debug::LogError("failed to initialize glew(0x%x).", status);
		return false;
	}

	if (GLEW_ARB_debug_output) {
		GL::DebugMessageCallback(GLDebugMessageCallback, nullptr);
		GL::Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	GL::ClearDepth(1);
	GL::DepthRange(0, 1);

	GL::Enable(GL_DEPTH_TEST);
	GL::DepthFunc(GL_LEQUAL);

	GL::Enable(GL_CULL_FACE);
	GL::CullFace(GL_BACK);

	GL::DepthMask(GL_TRUE);

	return true;
}

bool OpenGLDriver::IsSupported(const char* feature) {
	return !!glewIsSupported(feature);
}

const char* OpenGLDriver::GetVendor() {
	const char* answer = (const char*)GL::GetString(GL_VENDOR);
	if (answer == nullptr) { answer = "Unknown"; }
	return answer;
}

const char* OpenGLDriver::GetRenderer() {
	const char* answer = (const char*)GL::GetString(GL_RENDERER);
	if (answer == nullptr) { answer = "Unknown"; }
	return answer;
}

const char* OpenGLDriver::GetOpenGLVersion() {
	const char* answer = (const char*)GL::GetString(GL_VERSION);
	if (answer == nullptr) { answer = "Unknown"; }
	return answer;
}

const char* OpenGLDriver::GetGLSLVersion() {
	const char* answer = (const char*)GL::GetString(GL_SHADING_LANGUAGE_VERSION);
	if (answer == nullptr) { answer = "Unknown"; }
	return answer;
}

static void _STDCALL GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
	// Shader.
	if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB) {
		return;
	}

	std::string text = "OpenGL Debug Output message:\n";

	if (source == GL_DEBUG_SOURCE_API_ARB)					text += "Source: API.\n";
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)	text += "Source: WINDOW_SYSTEM.\n";
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)	text += "Source: SHADER_COMPILER.\n";
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)		text += "Source: THIRD_PARTY.\n";
	else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)		text += "Source: APPLICATION.\n";
	else if (source == GL_DEBUG_SOURCE_OTHER_ARB)			text += "Source: OTHER.\n";

	if (type == GL_DEBUG_TYPE_ERROR_ARB)					text += "Type: ERROR.\n";
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)	text += "Type: DEPRECATED_BEHAVIOR.\n";
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)	text += "Type: UNDEFINED_BEHAVIOR.\n";
	else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB)			text += "Type: PORTABILITY.\n";
	else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB)			text += "Type: PERFORMANCE.\n";
	else if (type == GL_DEBUG_TYPE_OTHER_ARB)				text += "Type: OTHER.\n";

	if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)				text += "Severity: HIGH.\n";
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)		text += "Severity: MEDIUM.\n";
	else if (severity == GL_DEBUG_SEVERITY_LOW_ARB)			text += "Severity: LOW.\n";

	text += message;

	if (severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
		Debug::LogError(text.c_str());
	}
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB) {
		Debug::LogWarning(text.c_str());
	}
	else {
		Debug::Log(text.c_str());
	}
}
