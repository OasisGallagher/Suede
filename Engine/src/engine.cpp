#include <gl/glew.h>

#include "engine.h"
#include "tools/debug.h"

extern Time timeInstance;
extern World worldInstance;
extern Screen screenInstance;
extern Logger loggerInstance;
extern Graphics graphicsInstance;

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

Engine::Engine() {
}

Engine* Engine::get() {
	static Engine instance;
	return &instance;
}

bool Engine::initialize() {
	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		AssertX(false, "failed to initialize glew.");
		return false;
	}

	if (GLEW_ARB_debug_output) {
		glDebugMessageCallbackARB(GLDebugMessageCallback, nullptr);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	}

	return true;
}

void Engine::release() {
}

void Engine::setLogReceiver(EngineLogReceiver* receiver) {
	Debug::SetLogReceiver(receiver);
}

void Engine::resize(int w, int h) {
	screen()->SetContentSize(w, h);
	glViewport(0, 0, w, h);
}

World Engine::world() {
	return worldInstance;
}

Time Engine::time() {
	return timeInstance;
}

Logger Engine::logger() {
	return loggerInstance;
}

Screen Engine::screen() {
	return screenInstance;
}

Graphics Engine::graphics() {
	return graphicsInstance;
}

void Engine::update() {
	time()->Update();
	world()->Update();
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

	AssertX(severity != GL_DEBUG_SEVERITY_HIGH_ARB, text);

	if (severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
		Debug::LogError(text);
	}
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB) {
		Debug::LogWarning(text);
	}
	else {
		Debug::Log(text);
	}
}
