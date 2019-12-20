#include "buffer.h"
#include "context.h"

Buffer::Buffer(Context* context) : context_(context), old_(0), buffer_(0) {
	memset(&attribute_, 0, sizeof(attribute_));
}

Buffer::~Buffer() {
	Destroy();
}

void Buffer::Create(uint target, size_t size, const void* data, uint usage) {
	Destroy();
	context_->GenBuffers(1, &buffer_);
	attribute_.size = size;
	attribute_.target = target;
	attribute_.usage = usage;

	attribute_.data = new char[size];
	if (data != nullptr) {
		memcpy(attribute_.data, data, size);
	}

	Bind();
	context_->BufferData(target, size, data, usage);
	Unbind();
}

void Buffer::Destroy() {
	if (buffer_ != 0) {
		context_->DeleteBuffers(1, &buffer_);
		buffer_ = 0;
	}

	delete[] attribute_.data;
	memset(&attribute_, 0, sizeof(Attribute));
}

void Buffer::Bind() {
	uint pname = GetBindingName(attribute_.target);
	context_->GetIntegerv(pname, (int*)&old_);
	context_->BindBuffer(attribute_.target, buffer_);
}

void Buffer::Unbind() {
	context_->BindBuffer(attribute_.target, old_);
	old_ = 0;
}

void* Buffer::Map() {
	return attribute_.data;
	//Bind();
	//void* ptr = context_->MapBuffer(attribute_.target, GL_READ_ONLY);
	//Unbind();
	//return ptr;
}

void Buffer::Unmap() {
	//Bind();
	//context_->UnmapBuffer(attribute_.target);
	//Unbind();
}

void Buffer::Update(int offset, size_t size, const void* data) {
	Bind();
	context_->BufferData(attribute_.target, attribute_.size, nullptr, attribute_.usage);
	context_->BufferSubData(attribute_.target, offset, size, data);
	memcpy(attribute_.data + offset, data, size);
	Unbind();
}

uint Buffer::GetBindingName(uint target) {
	if (target == GL_ARRAY_BUFFER) { return GL_ARRAY_BUFFER_BINDING; }
	if (target == GL_UNIFORM_BUFFER) { return GL_UNIFORM_BUFFER_BINDING; }
	if (target == GL_TEXTURE_BUFFER) { return GL_TEXTURE_BUFFER_BINDING; }
	if (target == GL_ELEMENT_ARRAY_BUFFER) { return GL_ELEMENT_ARRAY_BUFFER_BINDING; }

	Debug::LogError("undefined target binding name");
	return 0;
}
