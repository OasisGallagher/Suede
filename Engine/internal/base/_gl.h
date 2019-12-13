#pragma once
#include <gl/glew.h>

#include "debug/debug.h"
#include "tools/string.h"

class _GL {
public:
	static void ActiveTexture(GLenum texture);
	static void AttachShader(GLuint program, GLuint shader);
	static void BeginQuery(GLenum target, GLuint id);
	static void BindAttribLocation(GLuint program, GLuint index, const GLchar* name);
	static void BindBuffer(GLenum target, GLuint buffer);
	static void BindBufferBase(GLenum target, GLuint index, GLuint buffer);
	static void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static void BindFramebuffer(GLenum target, GLuint framebuffer);
	static void BindRenderbuffer(GLenum target, GLuint renderbuffer);
	static void BindTexture(GLenum target, GLuint texture);
	static void BindVertexArray(GLuint array);
	static void BlendFunc(GLenum sfactor, GLenum dfactor);
	static void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	static void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
	static GLenum CheckFramebufferStatus(GLenum target);
	static void Clear(GLbitfield mask);
	static void ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	static void ClearDepth(GLclampd depth);
	static void ClearStencil(GLint s);
	static void CompileShader(GLuint shader);
	static GLuint CreateProgram();
	static GLuint CreateShader(GLenum type);
	static void CullFace(GLenum mode);
	static void DebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
	static void DeleteBuffers(GLsizei n, const GLuint* buffers);
	static void DeleteFramebuffers(GLsizei n, GLuint* framebuffers);
	static void DeleteProgram(GLuint program);
	static void DeleteQueries(GLsizei n, GLuint* ids);
	static void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
	static void DeleteShader(GLuint shader);
	static void DeleteTextures(GLsizei n, const GLuint* textures);
	static void DeleteVertexArrays(GLsizei n, const GLuint* arrays);
	static void DepthFunc(GLenum func);
	static void DepthMask(GLboolean flag);
	static void DepthRange(GLclampd zNear, GLclampd zFar);
	static void Disable(GLenum cap);
	static void DrawBuffers(GLsizei n, const GLenum* bufs);
	static void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, void* indices, GLint basevertex);
	static void DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex);
	static void Enable(GLenum cap);
	static void EnableVertexAttribArray(GLuint index);
	static void EndQuery(GLenum target);
	static void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static void FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static void GenBuffers(GLsizei n, GLuint* buffers);
	static void GenerateMipmap(GLenum target);
	static void GenFramebuffers(GLsizei n, GLuint* framebuffers);
	static void GenQueries(GLsizei n, GLuint* ids);
	static void GenRenderbuffers(GLsizei n, GLuint* renderbuffers);
	static void GenTextures(GLsizei n, GLuint* textures);
	static void GenVertexArrays(GLsizei n, GLuint* arrays);
	static void GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	static void GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
	static GLenum GetError();
	static void GetFloatv(GLenum pname, GLfloat* params);
	static void GetIntegerv(GLenum pname, GLint* params);
	static void GetProgramiv(GLuint program, GLenum pname, GLint* params);
	static void GetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params);
	static void GetQueryObjectiv(GLuint id, GLenum pname, GLint* params);
	static void GetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params);
	static void GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params);
	static void GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params);
	static void GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	static void GetShaderiv(GLuint shader, GLenum pname, GLint* params);
	static const GLubyte* GetString(GLenum name);
	static void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
	static void GetTexParameteriv(GLenum target, GLenum pname, GLint* params);
	static GLuint GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
	static GLint GetUniformLocation(GLuint program, const GLchar* name);
	static GLboolean IsEnabled(GLenum cap);
	static GLboolean IsTexture(GLuint texture);
	static void LinkProgram(GLuint program);
	static void* MapBuffer(GLenum target, GLenum access);
	static void PixelStorei(GLenum pname, GLint param);
	static void PolygonMode(GLenum face, GLenum mode);
	static void PolygonOffset(GLfloat factor, GLfloat units);
	static void ProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static void ProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value);
	static void ProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static void ProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static void ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static void ReadBuffer(GLenum mode);
	static void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
	static void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
	static void ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
	static void StencilFunc(GLenum func, GLint ref, GLuint mask);
	static void StencilMask(GLuint mask);
	static void StencilMaskSeparate(GLenum face, GLuint mask);
	static void StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
	static void TexBuffer(GLenum target, GLenum internalformat, GLuint buffer);
	static void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
	static void TexParameteri(GLenum target, GLenum pname, GLint param);
	static void UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	static void UnmapBuffer(GLenum target);
	static void UseProgram(GLuint program);
	static void ValidateProgram(GLuint program);
	static void VertexAttribDivisor(GLuint index, GLuint divisor);
	static void VertexAttribI1i(GLuint index, GLint x);
	static void VertexAttribI2i(GLuint index, GLint x, GLint y);
	static void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
	static void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
	static void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

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

inline void _GL::Verify(const char* func) {
#define CASE(errorEnum)		case errorEnum: message = #errorEnum; break
	GLenum error = glGetError();
	std::string message;
	switch (error) {
		case GL_NO_ERROR:
			break;
		CASE(GL_INVALID_ENUM);
		CASE(GL_INVALID_VALUE);
		CASE(GL_INVALID_OPERATION);
		CASE(GL_STACK_OVERFLOW);
		CASE(GL_STACK_UNDERFLOW);
		CASE(GL_OUT_OF_MEMORY);
		CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
		default: message = String::Format("undefined error 0x % x", error); break;
	}
#undef CASE

	if (!message.empty()) {
		Debug::LogError("%s: %s.", func, message.c_str());
	}
}

template <class T> inline
T _GL::VerifyR(const char* func, T ans) {
	Verify(func);
	return ans;
}

inline void _GL::ActiveTexture(GLenum texture) {
	GL_CALL(glActiveTexture(texture));
}

inline void _GL::AttachShader(GLuint program, GLuint shader) {
	GL_CALL(glAttachShader(program, shader));
}

inline void _GL::BeginQuery(GLenum target, GLuint id) {
	GL_CALL(glBeginQuery(target, id));
}

inline void _GL::BindAttribLocation(GLuint program, GLuint index, const GLchar* name) {
	GL_CALL(glBindAttribLocation(program, index, name));
}

inline void _GL::BindBuffer(GLenum target, GLuint buffer) {
	GL_CALL(glBindBuffer(target, buffer));
}

inline void _GL::BindBufferBase(GLenum target, GLuint index, GLuint buffer) {
	GL_CALL(glBindBufferBase(target, index, buffer));
}

inline void _GL::BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
	GL_CALL(glBindBufferRange(target, index, buffer, offset, size));
}

inline void _GL::BindFramebuffer(GLenum target, GLuint framebuffer) {
	GL_CALL(glBindFramebuffer(target, framebuffer));
}

inline void _GL::BindRenderbuffer(GLenum target, GLuint renderbuffer) {
	GL_CALL(glBindRenderbuffer(target, renderbuffer));
}

inline void _GL::BindTexture(GLenum target, GLuint texture) {
	GL_CALL(glBindTexture(target, texture));
}

inline void _GL::BindVertexArray(GLuint array) {
	GL_CALL(glBindVertexArray(array));
}

inline void _GL::BlendFunc(GLenum sfactor, GLenum dfactor) {
	GL_CALL(glBlendFunc(sfactor, dfactor));
}

inline void _GL::BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
	GL_CALL(glBufferData(target, size, data, usage));
}

inline void _GL::BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
	GL_CALL(glBufferSubData(target, offset, size, data));
}

inline GLenum _GL::CheckFramebufferStatus(GLenum target) {
	GL_CALL_R(glCheckFramebufferStatus(target));
}

inline void _GL::Clear(GLbitfield mask) {
	GL_CALL(glClear(mask));
}

inline void _GL::ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	GL_CALL(glClearColor(red, green, blue, alpha));
}

inline void _GL::ClearDepth(GLclampd depth) {
	GL_CALL(glClearDepth(depth));
}

inline void _GL::ClearStencil(GLint s) {
	GL_CALL(glClearStencil(s));
}

inline void _GL::CompileShader(GLuint shader) {
	GL_CALL(glCompileShader(shader));
}

inline GLuint _GL::CreateProgram() {
	GL_CALL_R(glCreateProgram());
}

inline GLuint _GL::CreateShader(GLenum type) {
	GL_CALL_R(glCreateShader(type));
}

inline void _GL::CullFace(GLenum mode) {
	GL_CALL(glCullFace(mode));
}

inline void _GL::DebugMessageCallback(GLDEBUGPROC callback, const void* userParam) {
	GL_CALL(glDebugMessageCallback(callback, userParam));
}

inline void _GL::DeleteBuffers(GLsizei n, const GLuint* buffers) {
	GL_CALL(glDeleteBuffers(n, buffers));
}

inline void _GL::DeleteFramebuffers(GLsizei n, GLuint* framebuffers) {
	GL_CALL(glDeleteFramebuffers(n, framebuffers));
}

inline void _GL::DeleteProgram(GLuint program) {
	GL_CALL(glDeleteProgram(program));
}

inline void _GL::DeleteQueries(GLsizei n, GLuint* ids) {
	GL_CALL(glDeleteQueries(n, ids));
}

inline void _GL::DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
	GL_CALL(glDeleteRenderbuffers(n, renderbuffers));
}

inline void _GL::DeleteShader(GLuint shader) {
	GL_CALL(glDeleteShader(shader));
}

inline void _GL::DeleteTextures(GLsizei n, const GLuint* textures) {
	GL_CALL(glDeleteTextures(n, textures));
}

inline void _GL::DeleteVertexArrays(GLsizei n, const GLuint* arrays) {
	GL_CALL(glDeleteVertexArrays(n, arrays));
}

inline void _GL::DepthFunc(GLenum func) {
	GL_CALL(glDepthFunc(func));
}

inline void _GL::DepthMask(GLboolean flag) {
	GL_CALL(glDepthMask(flag));
}

inline void _GL::DepthRange(GLclampd zNear, GLclampd zFar) {
	GL_CALL(glDepthRange(zNear, zFar));
}

inline void _GL::Disable(GLenum cap) {
	GL_CALL(glDisable(cap));
}

inline void _GL::DrawBuffers(GLsizei n, const GLenum* bufs) {
	GL_CALL(glDrawBuffers(n, bufs));
}

inline void _GL::DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, void* indices, GLint basevertex) {
	GL_CALL(glDrawElementsBaseVertex(mode, count, type, indices, basevertex));
}

inline void _GL::DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) {
	GL_CALL(glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex));
}

inline void _GL::Enable(GLenum cap) {
	GL_CALL(glEnable(cap));
}

inline void _GL::EnableVertexAttribArray(GLuint index) {
	GL_CALL(glEnableVertexAttribArray(index));
}

inline void _GL::EndQuery(GLenum target) {
	GL_CALL(glEndQuery(target));
}

inline void _GL::FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
	GL_CALL(glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer));
}

inline void _GL::FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) {
	GL_CALL(glFramebufferTexture(target, attachment, texture, level));
}

inline void _GL::GenBuffers(GLsizei n, GLuint* buffers) {
	GL_CALL(glGenBuffers(n, buffers));
}

inline void _GL::GenerateMipmap(GLenum target) {
	GL_CALL(glGenerateMipmap(target));
}

inline void _GL::GenFramebuffers(GLsizei n, GLuint* framebuffers) {
	GL_CALL(glGenFramebuffers(n, framebuffers));
}

inline void _GL::GenQueries(GLsizei n, GLuint* ids) {
	GL_CALL(glGenQueries(n, ids));
}

inline void _GL::GenRenderbuffers(GLsizei n, GLuint* renderbuffers) {
	GL_CALL(glGenRenderbuffers(n, renderbuffers));
}

inline void _GL::GenTextures(GLsizei n, GLuint* textures) {
	GL_CALL(glGenTextures(n, textures));
}

inline void _GL::GenVertexArrays(GLsizei n, GLuint* arrays) {
	GL_CALL(glGenVertexArrays(n, arrays));
}

inline void _GL::GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
	GL_CALL(glGetActiveUniform(program, index, bufSize, length, size, type, name));
}

inline void _GL::GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) {
	GL_CALL(glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params));
}

inline GLenum _GL::GetError() {
	GL_CALL_R(glGetError());
}

inline void _GL::GetFloatv(GLenum pname, GLfloat* params) {
	GL_CALL(glGetFloatv(pname, params));
}

inline void _GL::GetIntegerv(GLenum pname, GLint* params) {
	GL_CALL(glGetIntegerv(pname, params));
}

inline void _GL::GetProgramiv(GLuint program, GLenum pname, GLint* params) {
	GL_CALL(glGetProgramiv(program, pname, params));
}

inline void _GL::GetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params) {
	GL_CALL(glGetQueryObjecti64v(id, pname, params));
}

inline void _GL::GetQueryObjectiv(GLuint id, GLenum pname, GLint* params) {
	GL_CALL(glGetQueryObjectiv(id, pname, params));
}

inline void _GL::GetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params) {
	GL_CALL(glGetQueryObjectui64v(id, pname, params));
}

inline void _GL::GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params) {
	GL_CALL(glGetQueryObjectuiv(id, pname, params));
}

inline void _GL::GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params) {
	GL_CALL(glGetRenderbufferParameteriv(target, pname, params));
}

inline void _GL::GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
	GL_CALL(glGetShaderInfoLog(shader, bufSize, length, infoLog));
}

inline void _GL::GetShaderiv(GLuint shader, GLenum pname, GLint* params) {
	GL_CALL(glGetShaderiv(shader, pname, params));
}

inline const GLubyte* _GL::GetString(GLenum name) {
	GL_CALL_R(glGetString(name));
}

inline void _GL::GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
	GL_CALL(glGetTexImage(target, level, format, type, pixels));
}

inline void _GL::GetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
	GL_CALL(glGetTexParameteriv(target, pname, params));
}

inline GLuint _GL::GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName) {
	GL_CALL_R(glGetUniformBlockIndex(program, uniformBlockName));
}

inline GLint _GL::GetUniformLocation(GLuint program, const GLchar* name) {
	GL_CALL_R(glGetUniformLocation(program, name));
}

inline GLboolean _GL::IsEnabled(GLenum cap) {
	GL_CALL_R(glIsEnabled(cap));
}

inline GLboolean _GL::IsTexture(GLuint texture) {
	GL_CALL_R(glIsTexture(texture));
}

inline void _GL::LinkProgram(GLuint program) {
	GL_CALL(glLinkProgram(program));
}

inline void* _GL::MapBuffer(GLenum target, GLenum access) {
	GL_CALL_R(glMapBuffer(target, access));
}

inline void _GL::PixelStorei(GLenum pname, GLint param) {
	GL_CALL(glPixelStorei(pname, param));
}

inline void _GL::PolygonMode(GLenum face, GLenum mode) {
	GL_CALL(glPolygonMode(face, mode));
}

inline void _GL::PolygonOffset(GLfloat factor, GLfloat units) {
	GL_CALL(glPolygonOffset(factor, units));
}

inline void _GL::ProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
	GL_CALL(glProgramUniform1fv(program, location, count, value));
}

inline void _GL::ProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value) {
	GL_CALL(glProgramUniform1iv(program, location, count, value));
}

inline void _GL::ProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
	GL_CALL(glProgramUniform3fv(program, location, count, value));
}

inline void _GL::ProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
	GL_CALL(glProgramUniform4fv(program, location, count, value));
}

inline void _GL::ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	GL_CALL(glProgramUniformMatrix4fv(program, location, count, transpose, value));
}

inline void _GL::ReadBuffer(GLenum mode) {
	GL_CALL(glReadBuffer(mode));
}

inline void _GL::ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	GL_CALL(glReadPixels(x, y, width, height, format, type, pixels));
}

inline void _GL::RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_CALL(glRenderbufferStorage(target, internalformat, width, height));
}

inline void _GL::Scissor(GLint x, GLint y, GLsizei width, GLsizei height) {
	GL_CALL(glScissor(x, y, width, height));
}

inline void _GL::ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
	GL_CALL(glShaderSource(shader, count, string, length));
}

inline void _GL::StencilFunc(GLenum func, GLint ref, GLuint mask) {
	GL_CALL(glStencilFunc(func, ref, mask));
}

inline void _GL::StencilMask(GLuint mask) {
	GL_CALL(glStencilMask(mask));
}

inline void _GL::StencilMaskSeparate(GLenum face, GLuint mask) {
	GL_CALL(glStencilMaskSeparate(face, mask));
}

inline void _GL::StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
	GL_CALL(glStencilOp(sfail, dpfail, dppass));
}

inline void _GL::TexBuffer(GLenum target, GLenum internalformat, GLuint buffer) {
	GL_CALL(glTexBuffer(target, internalformat, buffer));
}

inline void _GL::TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
	GL_CALL(glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels));
}

inline void _GL::TexParameteri(GLenum target, GLenum pname, GLint param) {
	GL_CALL(glTexParameteri(target, pname, param));
}

inline void _GL::UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
	GL_CALL(glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding));
}

inline void _GL::UnmapBuffer(GLenum target) {
	GL_CALL(glUnmapBuffer(target));
}

inline void _GL::UseProgram(GLuint program) {
	GL_CALL(glUseProgram(program));
}

inline void _GL::ValidateProgram(GLuint program) {
	GL_CALL(glValidateProgram(program));
}

inline void _GL::VertexAttribDivisor(GLuint index, GLuint divisor) {
	GL_CALL(glVertexAttribDivisor(index, divisor));
}

inline void _GL::VertexAttribI1i(GLuint index, GLint x) {
	GL_CALL(glVertexAttribI1i(index, x));
}

inline void _GL::VertexAttribI2i(GLuint index, GLint x, GLint y) {
	GL_CALL(glVertexAttribI2i(index, x, y));
}

inline void _GL::VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
	GL_CALL(glVertexAttribIPointer(index, size, type, stride, pointer));
}

inline void _GL::VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
	GL_CALL(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
}

inline void _GL::Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
	GL_CALL(glViewport(x, y, width, height));
}

#undef GL_CALL
#undef GL_CALL_R