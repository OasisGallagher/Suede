#include <wrappers/gl.h>

#include "time2.h"
#include "world.h"
#include "engine.h"
#include "screen.h"
#include "resources.h"
#include "debug/debug.h"

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

bool Engine::Initialize() {
	setlocale(LC_ALL, "");
	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		Debug::LogError("failed to initialize glew.");
		return false;
	}

	if (GLEW_ARB_debug_output) {
		GL::DebugMessageCallback(GLDebugMessageCallback, nullptr);
		GL::Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	SetDefaultRenderStates();

	Resources::Import();
	return true;
}

void Engine::Release() {
	WorldInstance().reset();
}

void Engine::Resize(int w, int h) {
	Screen::Set(w, h);
	GL::Viewport(0, 0, w, h);
}

void Engine::Update() {
	Time::Update();
	WorldInstance()->Update();
}

void Engine::SetDefaultRenderStates() {
	GL::Enable(GL_DEPTH_TEST);
	GL::DepthFunc(GL_LEQUAL);

	GL::Enable(GL_CULL_FACE);
	GL::CullFace(GL_BACK);

	GL::DepthMask(GL_TRUE);
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
