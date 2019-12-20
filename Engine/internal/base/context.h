#pragma once
#include <thread>

#include "mesh.h"
#include "glenums.h"

enum class ContextLimitType {
	MaxColorAttachments,
	MaxUniformBlockSize,
	MaxUniformBufferBindings,

	MaxVertexAttribs,

	// https://www.khronos.org/opengl/wiki/Common_Mistakes#Texture_Unit
	// number of texture units was introduced as well which you can get with
	// you should not use the above because it will give a low number on modern GPUs.
	// then came the programmable GPU. There aren't texture units anymore. 
	// today, you have texture image units (TIU) which you can get with:
	// GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
	MaxTextureUnits,

	// limit of the number of textures that can be bound at any one time.
	// and this is the limit on the image unit to be passed to functions like glActiveTexture and glBindSampler.
	MaxCombinedTextureImageUnits,

	RedBits,
	GreenBits,
	BlueBits,
	AlphaBits,
	DepthBits,
	StencilBits,

	MaxClipPlanes,
	MaxTextureSize,
	MaxTextureBufferSize,

	_Count
};

class Context {
public:
	virtual ~Context() {}

public:
	static Context* GetCurrent();
	static bool SetCurrent(Context* value);

public:
	virtual void Update();
	virtual void CullingUpdate();

public:
	int GetLimit(ContextLimitType type);
	bool IsSupported(const char* feature);
	bool InThisThread() const { return threadId_ == std::this_thread::get_id(); }

	void DrawElementsBaseVertex(MeshTopology topology, const TriangleBias& bias);
	void DrawElementsInstancedBaseVertex(MeshTopology topology, const TriangleBias & bias, uint instance);

public:
	void ActiveTexture(uint texture);
	void AttachShader(uint program, uint shader);
	void BeginQuery(uint target, uint id);
	void BindAttribLocation(uint program, uint index, const char* name);
	void BindBuffer(uint target, uint buffer);
	void BindBufferBase(uint target, uint index, uint buffer);
	void BindBufferRange(uint target, uint index, uint buffer, ptrdiff_t offset, ptrdiff_t size);
	void BindFramebuffer(uint target, uint framebuffer);
	void BindRenderbuffer(uint target, uint renderbuffer);
	void BindTexture(uint target, uint texture);
	void BindVertexArray(uint array);
	void BlendFunc(uint sfactor, uint dfactor);
	void BufferData(uint target, ptrdiff_t size, const void* data, uint usage);
	void BufferSubData(uint target, ptrdiff_t offset, ptrdiff_t size, const void* data);
	uint CheckFramebufferStatus(uint target);
	void Clear(uint mask);
	void ClearColor(float red, float green, float blue, float alpha);
	void ClearDepth(double depth);
	void ClearStencil(int s);
	void CompileShader(uint shader);
	uint CreateProgram();
	uint CreateShader(uint type);
	void CullFace(uint mode);
	typedef void(__stdcall *GLDEBUGPROC)(uint source, uint type, uint id, uint severity, int length, const char* message, const void* userParam);
	void DebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
	void DeleteBuffers(int n, const uint* buffers);
	void DeleteFramebuffers(int n, uint* framebuffers);
	void DeleteProgram(uint program);
	void DeleteQueries(int n, uint* ids);
	void DeleteRenderbuffers(int n, const uint* renderbuffers);
	void DeleteShader(uint shader);
	void DeleteTextures(int n, const uint* textures);
	void DeleteVertexArrays(int n, const uint* arrays);
	void DepthFunc(uint func);
	void DepthMask(bool flag);
	void DepthRange(double zNear, double zFar);
	void Disable(uint cap);
	void DrawBuffers(int n, const uint* bufs);
	void DrawElementsBaseVertex(uint mode, int count, uint type, void* indices, int basevertex);
	void DrawElementsInstancedBaseVertex(uint mode, int count, uint type, const void* indices, int instancecount, int basevertex);
	void Enable(uint cap);
	void EnableVertexAttribArray(uint index);
	void EndQuery(uint target);
	void FramebufferRenderbuffer(uint target, uint attachment, uint renderbuffertarget, uint renderbuffer);
	void FramebufferTexture(uint target, uint attachment, uint texture, int level);
	void GenBuffers(int n, uint* buffers);
	void GenerateMipmap(uint target);
	void GenFramebuffers(int n, uint* framebuffers);
	void GenQueries(int n, uint* ids);
	void GenRenderbuffers(int n, uint* renderbuffers);
	void GenTextures(int n, uint* textures);
	void GenVertexArrays(int n, uint* arrays);
	void GetActiveUniform(uint program, uint index, int bufSize, int* length, int* size, uint* type, char* name);
	void GetActiveUniformBlockiv(uint program, uint uniformBlockIndex, uint pname, int* params);
	uint GetError();
	void GetFloatv(uint pname, float* params);
	void GetIntegerv(uint pname, int* params);
	void GetProgramiv(uint program, uint pname, int* params);
	void GetQueryObjecti64v(uint id, uint pname, int64* params);
	void GetQueryObjectiv(uint id, uint pname, int* params);
	void GetQueryObjectui64v(uint id, uint pname, uint64* params);
	void GetQueryObjectuiv(uint id, uint pname, uint* params);
	void GetRenderbufferParameteriv(uint target, uint pname, int* params);
	void GetShaderInfoLog(uint shader, int bufSize, int* length, char* infoLog);
	void GetShaderiv(uint shader, uint pname, int* params);
	const uchar* GetString(uint name);
	void GetTexImage(uint target, int level, uint format, uint type, void* pixels);
	void GetTexParameteriv(uint target, uint pname, int* params);
	uint GetUniformBlockIndex(uint program, const char* uniformBlockName);
	int GetUniformLocation(uint program, const char* name);
	bool IsEnabled(uint cap);
	bool IsTexture(uint texture);
	void LinkProgram(uint program);
	void* MapBuffer(uint target, uint access);
	void PixelStorei(uint pname, int param);
	void PolygonMode(uint face, uint mode);
	void PolygonOffset(float factor, float units);
	void ProgramUniform1fv(uint program, int location, int count, const float* value);
	void ProgramUniform1iv(uint program, int location, int count, const int* value);
	void ProgramUniform3fv(uint program, int location, int count, const float* value);
	void ProgramUniform4fv(uint program, int location, int count, const float* value);
	void ProgramUniformMatrix4fv(uint program, int location, int count, bool transpose, const float* value);
	void ReadBuffer(uint mode);
	void ReadPixels(int x, int y, int width, int height, uint format, uint type, void* pixels);
	void RenderbufferStorage(uint target, uint internalformat, int width, int height);
	void Scissor(int x, int y, int width, int height);
	void ShaderSource(uint shader, int count, const char* const* string, const int* length);
	void StencilFunc(uint func, int ref, uint mask);
	void StencilMask(uint mask);
	void StencilMaskSeparate(uint face, uint mask);
	void StencilOp(uint sfail, uint dpfail, uint dppass);
	void TexBuffer(uint target, uint internalformat, uint buffer);
	void TexImage2D(uint target, int level, int internalformat, int width, int height, int border, uint format, uint type, const void* pixels);
	void TexParameteri(uint target, uint pname, int param);
	void UniformBlockBinding(uint program, uint uniformBlockIndex, uint uniformBlockBinding);
	void UnmapBuffer(uint target);
	void UseProgram(uint program);
	void ValidateProgram(uint program);
	void VertexAttribDivisor(uint index, uint divisor);
	void VertexAttribI1i(uint index, int x);
	void VertexAttribI2i(uint index, int x, int y);
	void VertexAttribIPointer(uint index, int size, uint type, int stride, const void* pointer);
	void VertexAttribPointer(uint index, int size, uint type, bool normalized, int stride, const void* pointer);
	void Viewport(int x, int y, int width, int height);

protected:
	virtual void OnActive(bool active) {}
	virtual bool Initialize();

private:
	class Command {
	public:
		typedef std::function<void(int, const uint*)> action_type;

	public:
		Command(int nParameters, const uint* parameters, const action_type& f);
		~Command() { delete[] parameters_; }
		Command(Command&& other);

		Command(const Command& other) = delete;
		Command& operator=(const Command&) = delete;

	public:
		void operator()() { action_(nParameters_, parameters_); }

	private:
		int nParameters_;
		uint* parameters_;
		action_type action_;
	};

private:
	std::thread::id threadId_;

	int oglLimits_[(int)ContextLimitType::_Count] = { INT_MIN };
	std::vector<Command> commands_;
};
