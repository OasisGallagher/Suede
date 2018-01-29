#pragma once
#include <gl/glew.h>

#include "debug/debug.h"

class GL {
public:
	static GLenum GetError();
	static GLuint CreateProgram();
	static void Enable(GLenum cap);
	static void Disable(GLenum cap);
	static void ClearStencil(GLint s);
	static void CullFace(GLenum mode);
	static void DepthFunc(GLenum func);
	static void Clear(GLbitfield mask);
	static void ReadBuffer(GLenum mode);
	static void StencilMask(GLuint mask);
	static void DepthMask(GLboolean flag);
	static GLboolean IsEnabled(GLenum cap);
	static void UseProgram(GLuint program);
	static void ClearDepth(GLclampd depth);
	static void DeleteShader(GLuint shader);
	static GLuint CreateShader(GLenum type);
	static void LinkProgram(GLuint program);
	static void CompileShader(GLuint shader);
	static void GenerateMipmap(GLenum target);
	static const char* GetString(GLenum name);
	static void DeleteProgram(GLuint program);
	static void ActiveTexture(GLenum texture);
	static void BindVertexArray(GLuint array);
	static GLboolean IsTexture(GLuint texture);
	static void ValidateProgram(GLuint program);
	static void EnableVertexAttribArray(GLuint index);
	static void GenBuffers(GLsizei n, GLuint* buffers);
	static void PixelStorei(GLenum pname, GLint param);
	static GLenum CheckFramebufferStatus(GLenum target);
	static void BindBuffer(GLenum target, GLuint buffer);
	static void GenTextures(GLsizei n, GLuint* textures);
	static void GetIntegerv(GLenum pname, GLint* params);
	static void BlendFunc(GLenum sfactor, GLenum dfactor);
	static void BindTexture(GLenum target, GLuint texture);
	static void DrawBuffers(GLsizei n, const GLenum* bufs);
	static void GenVertexArrays(GLsizei n, GLuint* arrays);
	static void AttachShader(GLuint program, GLuint shader);
	static void StencilMaskSeparate(GLenum face, GLuint mask);
	static void DeleteBuffers(GLsizei n, const GLuint* buffers);
	static void GenFramebuffers(GLsizei n, GLuint* framebuffers);
	static void DeleteTextures(GLsizei n, const GLuint* textures);
	static void StencilFunc(GLenum func, GLint ref, GLuint mask);
	static void VertexAttribDivisor(GLuint index, GLuint divisor);
	static void GenRenderbuffers(GLsizei n, GLuint* renderbuffers);
	static void BindFramebuffer(GLenum target, GLuint framebuffer);
	static void DeleteVertexArrays(GLsizei n, const GLuint* arrays);
	static void BindRenderbuffer(GLenum target, GLuint renderbuffer);
	static void StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
	static void TexParameteri(GLenum target, GLenum pname, GLint param);
	static GLint GetUniformLocation(GLuint program, const GLchar* name);
	static void GetShaderiv(GLuint shader, GLenum pname, GLint* params);
	static void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
	static void GetProgramiv(GLuint program, GLenum pname, GLint* params);
	static void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
	static void GetTexParameteriv(GLenum target, GLenum pname, GLint* params);
	static void DebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
	static void BindAttribLocation(GLuint program, GLuint index, const GLchar *name);
	static void ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	static void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	static void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
	static void FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static void GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	static void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
	static void ProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value);
	static void ProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static void ProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static void ProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static void ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
	static void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
	static void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, void* indices, GLint basevertex);
	static void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
	static void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static void ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
	static void GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	static void DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex);
	static void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);

private:
	static void Verify(const char* func);

	template <class T>
	static T VerifyR(const char* func, T ans);
};

#ifdef _DEBUG
#define GL_CALL(expression)		expression; Verify(__func__)
#define GL_CALL_R(expression)	return VerifyR(__func__, expression)
#else
#define GL_CALL(expression)		expression
#define GL_CALL_R(expression)	return expression
#endif

inline void GL::Verify(const char* func) {
#define CASE(errorEnum)		case errorEnum: message = #errorEnum; break
	const char* message = nullptr;
	switch (glGetError()) {
		case GL_NO_ERROR:
			break;
		CASE(GL_INVALID_ENUM);
		CASE(GL_INVALID_VALUE);
		CASE(GL_INVALID_OPERATION);
		CASE(GL_STACK_OVERFLOW);
		CASE(GL_STACK_UNDERFLOW);
		CASE(GL_OUT_OF_MEMORY);
		default:
			message = "undefined error";
			break;
	}
#undef CASE

	if (message != nullptr) {
		Debug::LogError("%s: %s.", func, message);
	}
}

template <class T> inline
T GL::VerifyR(const char* func, T ans) {
	Verify(func);
	return ans;
}

inline void GL::ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	GL_CALL(glReadPixels(x, y, width, height, format, type, pixels));
}

inline void GL::GetIntegerv(GLenum pname, GLint* params) {
	GL_CALL(glGetIntegerv(pname, params));
}

inline void GL::BindFramebuffer(GLenum target, GLuint framebuffer) {
	GL_CALL(glBindFramebuffer(target, framebuffer));
}

inline void GL::Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
	GL_CALL(glViewport(x, y, width, height));
}

inline void GL::ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	GL_CALL(glClearColor(red, green, blue, alpha));
}

inline void GL::Clear(GLbitfield mask) {
	GL_CALL(glClear(mask));
}

inline void GL::ReadBuffer(GLenum mode) {
	GL_CALL(glReadBuffer(mode));
}

inline void GL::StencilMask(GLuint mask) {
	GL_CALL(glStencilMask(mask));
}

inline void GL::DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
	GL_CALL(glDeleteRenderbuffers(n, renderbuffers));
}

inline void GL::GetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
	GL_CALL(glGetTexParameteriv(target, pname, params));
}

inline void GL::GenFramebuffers(GLsizei n, GLuint* framebuffers) {
	GL_CALL(glGenFramebuffers(n, framebuffers));
}

inline void GL::DrawBuffers(GLsizei n, const GLenum* bufs) {
	GL_CALL(glDrawBuffers(n, bufs));
}

inline void GL::GenRenderbuffers(GLsizei n, GLuint* renderbuffers) {
	GL_CALL(glGenRenderbuffers(n, renderbuffers));
}

inline void GL::BindRenderbuffer(GLenum target, GLuint renderbuffer) {
	GL_CALL(glBindRenderbuffer(target, renderbuffer));
}

inline void GL::RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_CALL(glRenderbufferStorage(target, internalformat, width, height));
}

inline void GL::FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
	GL_CALL(glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer));
}

inline GLenum GL::CheckFramebufferStatus(GLenum target) {
	GL_CALL_R(glCheckFramebufferStatus(target));
}

inline void GL::UseProgram(GLuint program) {
	GL_CALL(glUseProgram(program));
}

inline GLenum GL::GetError() {
	GL_CALL_R(glGetError());
}

inline void GL::DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, void* indices, GLint basevertex) {
	GL_CALL(glDrawElementsBaseVertex(mode, count, type, indices, basevertex));
}

inline void GL::DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) {
	GL_CALL(glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex));
}

inline GLboolean GL::IsEnabled(GLenum cap) {
	GL_CALL_R(glIsEnabled(cap));
}

inline void GL::CullFace(GLenum mode) {
	GL_CALL(glCullFace(mode));
}

inline void GL::DepthFunc(GLenum func) {
	GL_CALL(glDepthFunc(func));
}

inline void GL::DepthMask(GLboolean flag) {
	GL_CALL(glDepthMask(flag));
}

inline void GL::BlendFunc(GLenum sfactor, GLenum dfactor) {
	GL_CALL(glBlendFunc(sfactor, dfactor));
}

inline void GL::Disable(GLenum cap) {
	GL_CALL(glDisable(cap));
}

inline GLuint GL::CreateProgram() {
	GL_CALL_R(glCreateProgram());
}

inline void GL::DeleteProgram(GLuint program) {
	GL_CALL(glDeleteProgram(program));
}

inline void GL::GetShaderiv(GLuint shader, GLenum pname, GLint* params) {
	GL_CALL(glGetShaderiv(shader, pname, params));
}

inline void GL::FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) {
	GL_CALL(glFramebufferTexture(target, attachment, texture, level));
}

inline void GL::GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
	GL_CALL(glGetShaderInfoLog(shader, bufSize, length, infoLog));
}

inline void GL::LinkProgram(GLuint program) {
	GL_CALL(glLinkProgram(program));
}

inline void GL::GetProgramiv(GLuint program, GLenum pname, GLint* params) {
	GL_CALL(glGetProgramiv(program, pname, params));
}

inline void GL::ValidateProgram(GLuint program) {
	GL_CALL(glValidateProgram(program));
}

inline void GL::DeleteShader(GLuint shader) {
	GL_CALL(glDeleteShader(shader));
}

inline void GL::GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
	GL_CALL(glGetActiveUniform(program, index, bufSize, length, size, type, name));
}

inline GLint GL::GetUniformLocation(GLuint program, const GLchar* name) {
	GL_CALL_R(glGetUniformLocation(program, name));
}

inline void GL::ProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value) {
	GL_CALL(glProgramUniform1iv(program, location, count, value));
}

inline void GL::ProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
	GL_CALL(glProgramUniform1fv(program, location, count, value));
}

inline void GL::ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	GL_CALL(glProgramUniformMatrix4fv(program, location, count, transpose, value));
}

inline void GL::ProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
	GL_CALL(glProgramUniform3fv(program, location, count, value));
}

inline void GL::ProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
	GL_CALL(glProgramUniform4fv(program, location, count, value));
}

inline GLboolean GL::IsTexture(GLuint texture) {
	GL_CALL_R(glIsTexture(texture));
}

inline void GL::ActiveTexture(GLenum texture) {
	GL_CALL(glActiveTexture(texture));
}

inline void GL::BindTexture(GLenum target, GLuint texture) {
	GL_CALL(glBindTexture(target, texture));
}

inline void GL::DeleteTextures(GLsizei n, const GLuint* textures) {
	GL_CALL(glDeleteTextures(n, textures));
}

inline void GL::GenTextures(GLsizei n, GLuint* textures) {
	GL_CALL(glGenTextures(n, textures));
}

inline void GL::TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
	GL_CALL(glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels));
}

inline void GL::StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
	GL_CALL(glStencilOp(dppass, dppass, dppass));
}

inline void GL::TexParameteri(GLenum target, GLenum pname, GLint param) {
	GL_CALL(glTexParameteri(target, pname, param));
}

inline void GL::GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
	GL_CALL(glGetTexImage(target, level, format, type, pixels));
}

inline void GL::GenVertexArrays(GLsizei n, GLuint* arrays) {
	GL_CALL(glGenVertexArrays(n, arrays));
}

inline void GL::DeleteVertexArrays(GLsizei n, const GLuint* arrays) {
	GL_CALL(glDeleteVertexArrays(n, arrays));
}

inline void GL::GenBuffers(GLsizei n, GLuint* buffers) {
	GL_CALL(glGenBuffers(n, buffers));
}

inline void GL::PixelStorei(GLenum pname, GLint param) {
	GL_CALL(glPixelStorei(pname, param));
}

inline void GL::BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
	GL_CALL(glBufferData(target, size, data, usage));
}

inline void GL::EnableVertexAttribArray(GLuint index) {
	GL_CALL(glEnableVertexAttribArray(index));
}

inline void GL::VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
	GL_CALL(glVertexAttribIPointer(index, size, type, stride, pointer));
}

inline void GL::VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
	GL_CALL(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
}

inline void GL::StencilFunc(GLenum func, GLint ref, GLuint mask) {
	GL_CALL(glStencilFunc(func, ref, mask));
}

inline void GL::VertexAttribDivisor(GLuint index, GLuint divisor) {
	GL_CALL(glVertexAttribDivisor(index, divisor));
}

inline void GL::BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
	GL_CALL(glBufferSubData(target, offset, size, data));
}

inline void GL::StencilMaskSeparate(GLenum face, GLuint mask) {
	GL_CALL(glStencilMaskSeparate(face, mask));
}

inline void GL::DeleteBuffers(GLsizei n, const GLuint* buffers) {
	GL_CALL(glDeleteBuffers(n, buffers));
}

inline void GL::BindVertexArray(GLuint array) {
	GL_CALL(glBindVertexArray(array));
}

inline void GL::BindBuffer(GLenum target, GLuint buffer) {
	GL_CALL(glBindBuffer(target, buffer));
}

inline void GL::ClearDepth(GLclampd depth) {
	GL_CALL(glClearDepth(depth));
}

inline void GL::ClearStencil(GLint s) {
	GL_CALL(glClearStencil(s));
}

inline void GL::BindAttribLocation(GLuint program, GLuint index, const GLchar *name) {
	GL_CALL(glBindAttribLocation(program, index, name));
}

inline void GL::DebugMessageCallback(GLDEBUGPROC callback, const void* userParam) {
	GL_CALL(glDebugMessageCallback(callback, userParam));
}

inline void GL::Enable(GLenum cap) {
	GL_CALL(glEnable(cap));
}

inline GLuint GL::CreateShader(GLenum type) {
	GL_CALL_R(glCreateShader(type));
}

inline void GL::ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
	GL_CALL(glShaderSource(shader, count, string, length));
}

inline void GL::CompileShader(GLuint shader) {
	GL_CALL(glCompileShader(shader));
}

inline void GL::GenerateMipmap(GLenum target) {
	GL_CALL(glGenerateMipmap(target));
}

inline const char* GL::GetString(GLenum name) {
	GL_CALL_R((const char*)glGetString(name));
}

inline void GL::AttachShader(GLuint program, GLuint shader) {
	GL_CALL(glAttachShader(program, shader));
}


