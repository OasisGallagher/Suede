#include "uniformbuffer.h"

#include "buffer.h"
#include "context.h"

#include "memory/refptr.h"

uint UniformBuffer::bindingPoint_;

UniformBuffer::UniformBuffer(Context* context) : context_(context), ubo_(nullptr) {
}

UniformBuffer::~UniformBuffer() {
	Destroy();
}

bool UniformBuffer::Create(const std::string& name, uint size) {
	if (bindingPoint_ == context_->GetLimit(ContextLimitType::MaxUniformBufferBindings)) {
		Debug::LogError("too many uniform buffers");
		return false;
	}

	Initialize(name, size);
	return true;
}

void UniformBuffer::AttachBuffer(ShaderInternal* shader) {
	Attach(shader);
}

void UniformBuffer::AttachSubBuffer(ShaderInternal* shader, uint offset, uint size) {
	context_->BindBufferRange(GL_UNIFORM_BUFFER, binding_, ubo_->GetNativePointer(), offset, size);
	Attach(shader);
}

void UniformBuffer::AttachProgram(uint program) {
	const char* ptr = name_.c_str();
	const char* pos = strrchr(ptr, '[');
	std::string newName = name_;
	if (pos != nullptr) {
		newName.assign(ptr, pos);
	}

	uint index = context_->GetUniformBlockIndex(program, newName.c_str());
	if (index == GL_INVALID_INDEX) {
		return;
	}

	int dataSize = 0;
	context_->GetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
	if (dataSize != size_) {
		Debug::LogError("uniform buffer size mismatch");
		return;
	}
	
	context_->UniformBlockBinding(program, index, binding_);
}

bool UniformBuffer::UpdateBuffer(const void* data, uint offset, uint size) {
	if (size > context_->GetLimit(ContextLimitType::MaxUniformBlockSize)) {
		Debug::LogError("%d exceeds max buffer size.", size);
		return false;
	}

	ubo_->Update(offset, size, data);
	return true;
}

void UniformBuffer::Initialize(const std::string& name, uint size) {
	name_ = name;
	size_ = size;

	ubo_ = new Buffer(context_);
	ubo_->Create(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);

	binding_ = bindingPoint_++;
	context_->BindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_->GetNativePointer());
}

void UniformBuffer::Destroy() {
	delete ubo_;
}

void UniformBuffer::Attach(ShaderInternal* shader) {
	for (uint i = 0; i < shader->GetSubShaderCount(); ++i) {
		for (uint j = 0; j < shader->GetPassCount(i); ++j) {
			uint program = shader->GetNativePointer(i, j);
			AttachProgram(program);
		}
	}
}
