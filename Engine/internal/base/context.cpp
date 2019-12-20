#include <gl/glew.h>

#include "context.h"

#include "debug/debug.h"
#include "tools/string.h"
#include "internal/async/async.h"

static Context* current_;
static void __stdcall GLDebugMessageCallback(uint source, uint type, uint id, uint severity, int length, const char* message, const void* userParam);

bool Context::Initialize() {
	glewExperimental = true;
	GLenum status = glewInit();
	if (status != GLEW_OK) {
		Debug::LogError("failed to initialize glew(0x%x).", status);
		return false;
	}

	if (GLEW_ARB_debug_output) {
		DebugMessageCallback(GLDebugMessageCallback, nullptr);
		Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	ClearDepth(1);
	DepthRange(0, 1);

	Enable(GL_DEPTH_TEST);
	DepthFunc(GL_LEQUAL);

	Enable(GL_CULL_FACE);
	CullFace(GL_BACK);

	DepthMask(GL_TRUE);
	return true;
}

Context* Context::GetCurrent() { return current_; }
bool Context::SetCurrent(Context* value) {
	SUEDE_ASSERT(value != nullptr);

	if (current_ != nullptr) { current_->OnActive(false); }

	current_ = value;
	if (!(value->initialized_ |= value->Initialize())) {
		return false;
	}

	return true;
}

static void __stdcall GLDebugMessageCallback(uint source, uint type, uint id, uint severity, int length, const char* message, const void* userParam) {
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

#ifdef _DEBUG
#define GL_CALL(expression)		ZTHREAD_ASSERT_MAIN(); expression; Verify(__func__)
#define GL_CALL_R(expression)	return ZTHREAD_ASSERT_MAIN(), VerifyR(__func__, expression)
#else
#define GL_CALL(expression)		expression
#define GL_CALL_R(expression)	return expression
#endif

void Verify(const char* func) {
#define CASE(errorEnum)		case errorEnum: message = #errorEnum; break
	uint error = glGetError();
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
T VerifyR(const char* func, T ans) {
	return Verify(func), ans;
}

void Context::ActiveTexture(uint texture) {
	GL_CALL(glActiveTexture(texture));
}

void Context::AttachShader(uint program, uint shader) {
	GL_CALL(glAttachShader(program, shader));
}

void Context::BeginQuery(uint target, uint id) {
	GL_CALL(glBeginQuery(target, id));
}

void Context::BindAttribLocation(uint program, uint index, const char* name) {
	GL_CALL(glBindAttribLocation(program, index, name));
}

void Context::BindBuffer(uint target, uint buffer) {
	GL_CALL(glBindBuffer(target, buffer));
}

void Context::BindBufferBase(uint target, uint index, uint buffer) {
	GL_CALL(glBindBufferBase(target, index, buffer));
}

void Context::BindBufferRange(uint target, uint index, uint buffer, ptrdiff_t offset, ptrdiff_t size) {
	GL_CALL(glBindBufferRange(target, index, buffer, offset, size));
}

void Context::BindFramebuffer(uint target, uint framebuffer) {
	GL_CALL(glBindFramebuffer(target, framebuffer));
}

void Context::BindRenderbuffer(uint target, uint renderbuffer) {
	GL_CALL(glBindRenderbuffer(target, renderbuffer));
}

void Context::BindTexture(uint target, uint texture) {
	GL_CALL(glBindTexture(target, texture));
}

void Context::BindVertexArray(uint array) {
	GL_CALL(glBindVertexArray(array));
}

void Context::BlendFunc(uint sfactor, uint dfactor) {
	GL_CALL(glBlendFunc(sfactor, dfactor));
}

void Context::BufferData(uint target, ptrdiff_t size, const void* data, uint usage) {
	GL_CALL(glBufferData(target, size, data, usage));
}

void Context::BufferSubData(uint target, ptrdiff_t offset, ptrdiff_t size, const void* data) {
	GL_CALL(glBufferSubData(target, offset, size, data));
}

uint Context::CheckFramebufferStatus(uint target) {
	GL_CALL_R(glCheckFramebufferStatus(target));
}

void Context::Clear(uint mask) {
	GL_CALL(glClear(mask));
}

void Context::ClearColor(float red, float green, float blue, float alpha) {
	GL_CALL(glClearColor(red, green, blue, alpha));
}

void Context::ClearDepth(double depth) {
	GL_CALL(glClearDepth(depth));
}

void Context::ClearStencil(int s) {
	GL_CALL(glClearStencil(s));
}

void Context::CompileShader(uint shader) {
	GL_CALL(glCompileShader(shader));
}

uint Context::CreateProgram() {
	GL_CALL_R(glCreateProgram());
}

uint Context::CreateShader(uint type) {
	GL_CALL_R(glCreateShader(type));
}

void Context::CullFace(uint mode) {
	GL_CALL(glCullFace(mode));
}

void Context::DebugMessageCallback(GLDEBUGPROC callback, const void* userParam) {
	GL_CALL(glDebugMessageCallback(callback, userParam));
}

void Context::DepthFunc(uint func) {
	GL_CALL(glDepthFunc(func));
}

void Context::DepthMask(bool flag) {
	GL_CALL(glDepthMask(flag));
}

void Context::DepthRange(double zNear, double zFar) {
	GL_CALL(glDepthRange(zNear, zFar));
}

void Context::Disable(uint cap) {
	GL_CALL(glDisable(cap));
}

void Context::DrawBuffers(int n, const uint* bufs) {
	GL_CALL(glDrawBuffers(n, bufs));
}

void Context::DrawElementsInstancedBaseVertex(uint mode, int count, uint type, const void* indices, int instancecount, int basevertex) {
	GL_CALL(glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex));
}

void Context::Enable(uint cap) {
	GL_CALL(glEnable(cap));
}

void Context::EnableVertexAttribArray(uint index) {
	GL_CALL(glEnableVertexAttribArray(index));
}

void Context::EndQuery(uint target) {
	GL_CALL(glEndQuery(target));
}

void Context::FramebufferRenderbuffer(uint target, uint attachment, uint renderbuffertarget, uint renderbuffer) {
	GL_CALL(glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer));
}

void Context::FramebufferTexture(uint target, uint attachment, uint texture, int level) {
	GL_CALL(glFramebufferTexture(target, attachment, texture, level));
}

void Context::GenBuffers(int n, uint* buffers) {
	GL_CALL(glGenBuffers(n, buffers));
}

void Context::GenerateMipmap(uint target) {
	GL_CALL(glGenerateMipmap(target));
}

void Context::GenFramebuffers(int n, uint* framebuffers) {
	GL_CALL(glGenFramebuffers(n, framebuffers));
}

void Context::GenQueries(int n, uint* ids) {
	GL_CALL(glGenQueries(n, ids));
}

void Context::GenRenderbuffers(int n, uint* renderbuffers) {
	GL_CALL(glGenRenderbuffers(n, renderbuffers));
}

void Context::GenTextures(int n, uint* textures) {
	GL_CALL(glGenTextures(n, textures));
}

void Context::GenVertexArrays(int n, uint* arrays) {
	GL_CALL(glGenVertexArrays(n, arrays));
}

void Context::GetActiveUniform(uint program, uint index, int bufSize, int* length, int* size, uint* type, char* name) {
	GL_CALL(glGetActiveUniform(program, index, bufSize, length, size, type, name));
}

void Context::GetActiveUniformBlockiv(uint program, uint uniformBlockIndex, uint pname, int* params) {
	GL_CALL(glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params));
}

uint Context::GetError() {
	GL_CALL_R(glGetError());
}

void Context::GetFloatv(uint pname, float* params) {
	GL_CALL(glGetFloatv(pname, params));
}

void Context::GetIntegerv(uint pname, int* params) {
	GL_CALL(glGetIntegerv(pname, params));
}

void Context::GetProgramiv(uint program, uint pname, int* params) {
	GL_CALL(glGetProgramiv(program, pname, params));
}

void Context::GetQueryObjecti64v(uint id, uint pname, int64* params) {
	GL_CALL(glGetQueryObjecti64v(id, pname, params));
}

void Context::GetQueryObjectiv(uint id, uint pname, int* params) {
	GL_CALL(glGetQueryObjectiv(id, pname, params));
}

void Context::GetQueryObjectui64v(uint id, uint pname, uint64* params) {
	GL_CALL(glGetQueryObjectui64v(id, pname, params));
}

void Context::GetQueryObjectuiv(uint id, uint pname, uint* params) {
	GL_CALL(glGetQueryObjectuiv(id, pname, params));
}

void Context::GetRenderbufferParameteriv(uint target, uint pname, int* params) {
	GL_CALL(glGetRenderbufferParameteriv(target, pname, params));
}

void Context::GetShaderInfoLog(uint shader, int bufSize, int* length, char* infoLog) {
	GL_CALL(glGetShaderInfoLog(shader, bufSize, length, infoLog));
}

void Context::GetShaderiv(uint shader, uint pname, int* params) {
	GL_CALL(glGetShaderiv(shader, pname, params));
}

const uchar* Context::GetString(uint name) {
	GL_CALL_R(glGetString(name));
}

void Context::GetTexImage(uint target, int level, uint format, uint type, void* pixels) {
	GL_CALL(glGetTexImage(target, level, format, type, pixels));
}

void Context::GetTexParameteriv(uint target, uint pname, int* params) {
	GL_CALL(glGetTexParameteriv(target, pname, params));
}

uint Context::GetUniformBlockIndex(uint program, const char* uniformBlockName) {
	GL_CALL_R(glGetUniformBlockIndex(program, uniformBlockName));
}

int Context::GetUniformLocation(uint program, const char* name) {
	GL_CALL_R(glGetUniformLocation(program, name));
}

bool Context::IsEnabled(uint cap) {
	GL_CALL_R(!!glIsEnabled(cap));
}

bool Context::IsTexture(uint texture) {
	GL_CALL_R(!!glIsTexture(texture));
}

void Context::LinkProgram(uint program) {
	GL_CALL(glLinkProgram(program));
}

void* Context::MapBuffer(uint target, uint access) {
	GL_CALL_R(glMapBuffer(target, access));
}

void Context::PixelStorei(uint pname, int param) {
	GL_CALL(glPixelStorei(pname, param));
}

void Context::PolygonMode(uint face, uint mode) {
	GL_CALL(glPolygonMode(face, mode));
}

void Context::PolygonOffset(float factor, float units) {
	GL_CALL(glPolygonOffset(factor, units));
}

void Context::ProgramUniform1fv(uint program, int location, int count, const float* value) {
	GL_CALL(glProgramUniform1fv(program, location, count, value));
}

void Context::ProgramUniform1iv(uint program, int location, int count, const int* value) {
	GL_CALL(glProgramUniform1iv(program, location, count, value));
}

void Context::ProgramUniform3fv(uint program, int location, int count, const float* value) {
	GL_CALL(glProgramUniform3fv(program, location, count, value));
}

void Context::ProgramUniform4fv(uint program, int location, int count, const float* value) {
	GL_CALL(glProgramUniform4fv(program, location, count, value));
}

void Context::ProgramUniformMatrix4fv(uint program, int location, int count, bool transpose, const float* value) {
	GL_CALL(glProgramUniformMatrix4fv(program, location, count, transpose, value));
}

void Context::ReadBuffer(uint mode) {
	GL_CALL(glReadBuffer(mode));
}

void Context::ReadPixels(int x, int y, int width, int height, uint format, uint type, void* pixels) {
	GL_CALL(glReadPixels(x, y, width, height, format, type, pixels));
}

void Context::RenderbufferStorage(uint target, uint internalformat, int width, int height) {
	GL_CALL(glRenderbufferStorage(target, internalformat, width, height));
}

void Context::Scissor(int x, int y, int width, int height) {
	GL_CALL(glScissor(x, y, width, height));
}

void Context::ShaderSource(uint shader, int count, const char* const* string, const int* length) {
	GL_CALL(glShaderSource(shader, count, string, length));
}

void Context::StencilFunc(uint func, int ref, uint mask) {
	GL_CALL(glStencilFunc(func, ref, mask));
}

void Context::StencilMask(uint mask) {
	GL_CALL(glStencilMask(mask));
}

void Context::StencilMaskSeparate(uint face, uint mask) {
	GL_CALL(glStencilMaskSeparate(face, mask));
}

void Context::StencilOp(uint sfail, uint dpfail, uint dppass) {
	GL_CALL(glStencilOp(sfail, dpfail, dppass));
}

void Context::TexBuffer(uint target, uint internalformat, uint buffer) {
	GL_CALL(glTexBuffer(target, internalformat, buffer));
}

void Context::TexImage2D(uint target, int level, int internalformat, int width, int height, int border, uint format, uint type, const void* pixels) {
	GL_CALL(glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels));
}

void Context::TexParameteri(uint target, uint pname, int param) {
	GL_CALL(glTexParameteri(target, pname, param));
}

void Context::UniformBlockBinding(uint program, uint uniformBlockIndex, uint uniformBlockBinding) {
	GL_CALL(glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding));
}

void Context::UnmapBuffer(uint target) {
	GL_CALL(glUnmapBuffer(target));
}

void Context::UseProgram(uint program) {
	GL_CALL(glUseProgram(program));
}

void Context::ValidateProgram(uint program) {
	GL_CALL(glValidateProgram(program));
}

void Context::VertexAttribDivisor(uint index, uint divisor) {
	GL_CALL(glVertexAttribDivisor(index, divisor));
}

void Context::VertexAttribI1i(uint index, int x) {
	GL_CALL(glVertexAttribI1i(index, x));
}

void Context::VertexAttribI2i(uint index, int x, int y) {
	GL_CALL(glVertexAttribI2i(index, x, y));
}

void Context::VertexAttribIPointer(uint index, int size, uint type, int stride, const void* pointer) {
	GL_CALL(glVertexAttribIPointer(index, size, type, stride, pointer));
}

void Context::VertexAttribPointer(uint index, int size, uint type, bool normalized, int stride, const void* pointer) {
	GL_CALL(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
}

void Context::Viewport(int x, int y, int width, int height) {
	GL_CALL(glViewport(x, y, width, height));
}

int Context::GetLimit(ContextLimitType type) {
#define GL_INTEGER(lim, name)	if (int value = 1) { GetIntegerv(name, &value); oglLimits_[(int)lim] = value; } else (void)0
	if (oglLimits_[0] == INT_MIN) {
		GL_INTEGER(ContextLimitType::MaxColorAttachments, GL_MAX_COLOR_ATTACHMENTS);
		GL_INTEGER(ContextLimitType::MaxTextureUnits, GL_MAX_TEXTURE_UNITS);
		GL_INTEGER(ContextLimitType::MaxCombinedTextureImageUnits, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		GL_INTEGER(ContextLimitType::MaxUniformBufferBindings, GL_MAX_UNIFORM_BUFFER_BINDINGS);
		GL_INTEGER(ContextLimitType::MaxVertexAttribs, GL_MAX_VERTEX_ATTRIBS);
		GL_INTEGER(ContextLimitType::MaxUniformBlockSize, GL_MAX_UNIFORM_BLOCK_SIZE);
		GL_INTEGER(ContextLimitType::RedBits, GL_RED_BITS);
		GL_INTEGER(ContextLimitType::GreenBits, GL_GREEN_BITS);
		GL_INTEGER(ContextLimitType::BlueBits, GL_BLUE_BITS);
		GL_INTEGER(ContextLimitType::AlphaBits, GL_ALPHA_BITS);
		GL_INTEGER(ContextLimitType::DepthBits, GL_DEPTH_BITS);
		GL_INTEGER(ContextLimitType::StencilBits, GL_STENCIL_BITS);
		GL_INTEGER(ContextLimitType::MaxClipPlanes, GL_MAX_CLIP_PLANES);
		GL_INTEGER(ContextLimitType::MaxTextureSize, GL_MAX_TEXTURE_SIZE);
		GL_INTEGER(ContextLimitType::MaxTextureBufferSize, GL_MAX_TEXTURE_BUFFER_SIZE);
	}
#undef GL_INTEGER

	SUEDE_VERIFY_INDEX((int)type, (int)ContextLimitType::_Count, 0);
	return oglLimits_[(int)type];
}

bool Context::IsSupported(const char* feature) {
	return !!glewIsSupported(feature);
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

void Context::DrawElementsBaseVertex(uint mode, int count, uint type, void* indices, int basevertex) {
	GL_CALL(glDrawElementsBaseVertex(mode, count, type, indices, basevertex));
}

void Context::DrawElementsBaseVertex(MeshTopology topology, const TriangleBias& bias) {
	DrawElementsBaseVertex(TopologyToGLEnum(topology), bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
}

void Context::DrawElementsInstancedBaseVertex(MeshTopology topology, const TriangleBias & bias, uint instance) {
	DrawElementsInstancedBaseVertex(TopologyToGLEnum(topology), bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), instance, bias.baseVertex);
}

class Destroyer {
public:
	Destroyer(GLsizei n, const GLuint* buffers, const std::function<void(GLsizei, const GLuint*)>& f) : n_(n), f_(f) {
		buffers_ = new GLuint[n];
		memcpy(buffers_, buffers, n * sizeof(GLuint));
	}

	~Destroyer() {
		delete[] buffers_;
	}

	void operator()() {
		f_(n_, buffers_);
	}

	Destroyer(Destroyer&& other) {
		n_ = other.n_;
		f_ = other.f_;
		buffers_ = other.buffers_;

		other.n_ = 0;
		other.buffers_ = nullptr;
		other.f_ = std::function<void(GLsizei, const GLuint*)>();
	}

	Destroyer(const Destroyer& other) = delete;
	Destroyer& operator=(const Destroyer&) = delete;

private:
	GLsizei n_;
	std::function<void(GLsizei, const GLuint*)> f_;
	GLuint* buffers_;
};

// TODO destroy gl resources in rendering thread.
static std::vector<Destroyer> delayDestroyers;

//void Context::Update() {
//	for (auto& destroyer : delayDestroyers) {
//		destroyer();
//	}
//
//	delayDestroyers.clear();
//}

void Context::DeleteBuffers(GLsizei n, const GLuint* buffers) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteBuffers(n, buffers));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, buffers, [](GLsizei n, const GLuint* buffers) {
			glDeleteBuffers(n, buffers);
		}));
	}
}

void Context::DeleteFramebuffers(GLsizei n, GLuint* framebuffers) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteFramebuffers(n, framebuffers));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, framebuffers, [](GLsizei n, const GLuint* buffers) {
			glDeleteFramebuffers(n, buffers);
		}));
	}
}

void Context::DeleteProgram(GLuint program) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteProgram(program));
	}
	else {
		delayDestroyers.push_back(Destroyer(1, &program, [](GLsizei n, const GLuint* buffers) {
			glDeleteProgram(*buffers);
		}));
	}
}

void Context::DeleteQueries(GLsizei n, GLuint* ids) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteQueries(n, ids));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, ids, [](GLsizei n, const GLuint* buffers) {
			glDeleteQueries(n, buffers);
		}));
	}
}

void Context::DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteRenderbuffers(n, renderbuffers));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, renderbuffers, [](GLsizei n, const GLuint* buffers) {
			glDeleteRenderbuffers(n, buffers);
		}));
	}
}

void Context::DeleteShader(GLuint shader) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteShader(shader));
	}
	else {
		delayDestroyers.push_back(Destroyer(1, &shader, [](GLsizei n, const GLuint* buffers) {
			glDeleteShader(*buffers);
		}));
	}
}

void Context::DeleteTextures(GLsizei n, const GLuint* textures) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteTextures(n, textures));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, textures, [](GLsizei n, const GLuint* buffers) {
			glDeleteTextures(n, buffers);
		}));
	}
}

void Context::DeleteVertexArrays(GLsizei n, const GLuint* arrays) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteVertexArrays(n, arrays));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, arrays, [](GLsizei n, const GLuint* buffers) {
			glDeleteVertexArrays(n, buffers);
		}));
	}
}
