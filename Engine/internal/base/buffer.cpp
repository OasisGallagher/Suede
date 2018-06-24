#include "buffer.h"

void Buffer::Create(GLenum target, size_t size, const void* data, GLenum usage) {
	Destroy();
	GL::GenBuffers(1, &buffer_);
	attribute_.size = size;
	attribute_.target = target;
	attribute_.usage = usage;

	Bind();
	GL::BufferData(target, size, data, usage);
	Unbind();
}

void Buffer::Destroy() {
	if (buffer_ != 0) {
		GL::DeleteBuffers(1, &buffer_);
		buffer_ = 0;
	}

	memset(&attribute_, 0, sizeof(Attribute));
}

void Buffer::Bind() {
	GLenum pname = GetBindingName(attribute_.target);
	GL::GetIntegerv(pname, (GLint*)&old_);
	GL::BindBuffer(attribute_.target, buffer_);
}

void Buffer::Unbind() {
	GL::BindBuffer(attribute_.target, old_);
	old_ = 0;
}

void * Buffer::Map() {
	Bind();
	void* ptr = GL::MapBuffer(attribute_.target, GL_READ_ONLY);
	Unbind();
	return ptr;
}

void Buffer::Unmap() {
	Bind();
	GL::UnmapBuffer(attribute_.target);
	Unbind();
}

void Buffer::Update(int offset, size_t size, const void* data) {
	Bind();
	GL::BufferData(attribute_.target, attribute_.size, nullptr, attribute_.usage);
	GL::BufferSubData(attribute_.target, offset, size, data);
	Unbind();
}

GLenum Buffer::GetBindingName(GLenum target) {
	if (target == GL_ARRAY_BUFFER) { return GL_ARRAY_BUFFER_BINDING; }
	if (target == GL_UNIFORM_BUFFER) { return GL_UNIFORM_BUFFER_BINDING; }
	if (target == GL_TEXTURE_BUFFER) { return GL_TEXTURE_BUFFER_BINDING; }
	if (target == GL_ELEMENT_ARRAY_BUFFER) { return GL_ELEMENT_ARRAY_BUFFER_BINDING; }

	Debug::LogError("undefined target binding name");
	return 0;
}
