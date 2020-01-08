#include "buffer.h"
#include "context.h"

Buffer::Buffer(Context* context) : context_(context), old_(0), buffer_(0) {
}

Buffer::~Buffer() {
	Destroy();
}

void Buffer::Create(uint target, size_t size, const void* data, uint usage) {
	Destroy();
	context_->GenBuffers(1, &buffer_);

	usage_ = usage;
	target_ = target;

	shadowDataSize_ = size;
	shadowData_.reset(new uchar[size]);
	if (data != nullptr) {
		memcpy(shadowData_.get(), data, size);
	}
	else {
		memset(shadowData_.get(), 0, shadowDataSize_);
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

	shadowData_ = nullptr;
	shadowDataSize_ = 0;
	usage_ = target_ = 0;
}

void Buffer::Bind() {
	uint pname = GetBindingName(target_);
	context_->GetIntegerv(pname, (int*)&old_);
	context_->BindBuffer(target_, buffer_);
}

void Buffer::Unbind() {
	context_->BindBuffer(target_, old_);
	old_ = 0;
}

void* Buffer::Map() {
	Bind();
	void* ptr = context_->MapBuffer(target_, GL_READ_WRITE);
	Unbind();
	return ptr;
}

void Buffer::Unmap() {
	Bind();
	context_->UnmapBuffer(target_);
	Unbind();
}

void Buffer::Update(int offset, size_t size, const void* data) {
	SUEDE_ASSERT(offset >= 0);
	SUEDE_ASSERT(offset + size <= shadowDataSize_);

	Bind();
	if (offset == 0 && shadowDataSize_ == size) {
		context_->BufferData(target_, shadowDataSize_, nullptr, usage_);
	}

	context_->BufferSubData(target_, offset, size, data);
	memcpy(shadowData_.get() + offset, data, size);
	Unbind();
}

uint Buffer::GetNativePointer() const {
	SUEDE_ASSERT(buffer_ != 0);
	return buffer_;
}

uint Buffer::GetBindingName(uint target) {
	if (target == GL_ARRAY_BUFFER) { return GL_ARRAY_BUFFER_BINDING; }
	if (target == GL_UNIFORM_BUFFER) { return GL_UNIFORM_BUFFER_BINDING; }
	if (target == GL_TEXTURE_BUFFER) { return GL_TEXTURE_BUFFER_BINDING; }
	if (target == GL_ELEMENT_ARRAY_BUFFER) { return GL_ELEMENT_ARRAY_BUFFER_BINDING; }

	Debug::LogError("undefined target binding name");
	return 0;
}
