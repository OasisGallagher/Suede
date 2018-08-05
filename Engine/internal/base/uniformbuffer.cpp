#include "buffer.h"
#include "../api/glutils.h"
#include "uniformbuffer.h"
#include "memory/memory.h"

uint UniformBuffer::bindingPoint_;

UniformBuffer::UniformBuffer() : ubo_(nullptr) {
}

UniformBuffer::~UniformBuffer() {
	Destroy();
}

bool UniformBuffer::Create(const std::string& name, uint size) {
	if (bindingPoint_ == GLUtils::GetLimits(GLLimitsMaxUniformBufferBindings)) {
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
	GL::BindBufferRange(GL_UNIFORM_BUFFER, binding_, ubo_->GetNativePointer(), offset, size);
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
	ubo_->Update(offset, size, data);
}

void UniformBuffer::Initialize(const std::string& name, uint size) {
	name_ = name;
	size_ = size;

	ubo_ = MEMORY_NEW(Buffer);
	ubo_->Create(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);

	binding_ = bindingPoint_++;
	GL::BindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_->GetNativePointer());
}

void UniformBuffer::Destroy() {
	MEMORY_DELETE(ubo_);
}

void UniformBuffer::Attach(Shader shader) {
	for (uint i = 0; i < shader->GetSubShaderCount(); ++i) {
		for (uint j = 0; j < shader->GetPassCount(i); ++j) {
			uint program = shader->GetNativePointer(i, j);
			AttachProgram(program);
		}
	}
}
