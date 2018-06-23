#include "api/glutils.h"
#include "uniformbuffer.h"
#include "memory/memory.h"

uint UniformBuffer::bindingPoint_;

UniformBuffer::UniformBuffer() : ubo_(0) {
}

UniformBuffer::~UniformBuffer() {
	Destroy();
}

bool UniformBuffer::Create(const std::string& name, uint size) {
	if (bindingPoint_ == GLLimits::Get(GLLimitsMaxUniformBufferBindings)) {
		Debug::LogError("too many uniform buffers");
		return false;
	}

	Initialize(name, size);
	return true;
}

void UniformBuffer::AttachBuffer(Shader shader) {
	Attach(shader);
}

void UniformBuffer::AttachSubBuffer(Shader shader, uint offset, uint size) {
	GL::BindBufferRange(GL_UNIFORM_BUFFER, binding_, ubo_, offset, size);
	Attach(shader);
}

void UniformBuffer::AttachProgram(uint program) {
	const char* ptr = name_.c_str();
	const char* pos = strrchr(ptr, '[');
	std::string newName = name_;
	if (pos != nullptr) {
		newName.assign(ptr, pos);
	}
	GLuint index = GL::GetUniformBlockIndex(program, newName.c_str());

	if (index == GL_INVALID_INDEX) {
		return;
	}

	int dataSize = 0;
	GL::GetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
	if (dataSize != size_) {
		Debug::LogError("uniform buffer size mismatch");
		return;
	}
	
	GL::UniformBlockBinding(program, index, binding_);
}

void UniformBuffer::UpdateBuffer(const void* data, uint offset, uint size) {
	Bind();
	GL::BufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
	GL::BufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	Unbind();
}

void UniformBuffer::Bind() {
	GL::GetIntegerv(GL_UNIFORM_BUFFER_BINDING, (GLint*)&oldUbo_);
	GL::BindBuffer(GL_UNIFORM_BUFFER, ubo_);
}

void UniformBuffer::Unbind() {
	GL::BindBuffer(GL_UNIFORM_BUFFER, oldUbo_);
	oldUbo_ = 0;
}

void UniformBuffer::Initialize(const std::string& name, uint size) {
	name_ = name;
	size_ = size;

	GL::GenBuffers(1, &ubo_);
	Bind();
	GL::BufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
	Unbind();

	binding_ = bindingPoint_++;
	GL::BindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_);
}

void UniformBuffer::Destroy() {
	if (oldUbo_ != 0) {
		GL::DeleteBuffers(1, &oldUbo_);
	}
}

void UniformBuffer::Attach(Shader shader) {
	for (uint i = 0; i < shader->GetSubShaderCount(); ++i) {
		for (uint j = 0; j < shader->GetPassCount(i); ++j) {
			uint program = shader->GetNativePointer(i, j);
			AttachProgram(program);
		}
	}
}
