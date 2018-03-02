#include "ubo.h"
#include "memory/memory.h"

static uint bindingPoint;
static uint maxBindingPoints;

UBO::UBO() : ubo_(0) {
	if (maxBindingPoints == 0) {
		GL::GetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, (GLint*)&maxBindingPoints);
	}
}

UBO::~UBO() {
	Destroy();
}

bool UBO::Create(const std::string& name, uint size) {
	if (bindingPoint == maxBindingPoints) {
		Debug::LogError("too many uniform buffers");
		return false;
	}

	Initialize(name, size);
	return true;
}

void UBO::Attach(Shader shader) {
	for (uint i = 0; i < shader->GetSubShaderCount(); ++i) {
		for (uint j = 0; j < shader->GetPassCount(i); ++j) {
			uint program = shader->GetNativePointer(i, j);
			AttachToProgram(program);
		}
	}
}

void UBO::AttachToProgram(uint program) {
	GLuint index = GL::GetUniformBlockIndex(program, name_.c_str());
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

void UBO::SetBuffer(const void* data, uint offset, uint size) {
	Bind();
	GL::BufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	Unbind();
}

void UBO::Bind() {
	GL::GetIntegerv(GL_UNIFORM_BUFFER_BINDING, (GLint*)&oldUbo_);
	GL::BindBuffer(GL_UNIFORM_BUFFER, ubo_);
}

void UBO::Unbind() {
	GL::BindBuffer(GL_UNIFORM_BUFFER, oldUbo_);
}

void UBO::Initialize(const std::string& name, uint size) {
	name_ = name;
	size_ = size;

	GL::GenBuffers(1, &ubo_);
	Bind();
	GL::BufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	Unbind();

	binding_ = bindingPoint++;
	GL::BindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_);
}

void UBO::Destroy() {
	if (oldUbo_ != 0) {
		GL::DeleteBuffers(1, &oldUbo_);
	}
}
