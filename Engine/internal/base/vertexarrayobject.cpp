#include "tools/debug.h"
#include "vertexarrayobject.h"
#include "internal/memory/memory.h"

VertexArrayObject::VertexArrayObject() 
	: vao_(0), oldVao_(0), vbos_(nullptr), attributes_(nullptr), oldBuffer_(0)
	, bufferCount_(0) {
}

VertexArrayObject::~VertexArrayObject() {
	Destroy();
}

void VertexArrayObject::Create(size_t n) {
	Destroy();

	glGenVertexArrays(1, &vao_);
	Bind();
	
	vbos_ = Memory::CreateArray<GLuint>(n);
	glGenBuffers(n, vbos_);

	attributes_ = Memory::CreateArray<VBOAttribute>(n);

	bufferCount_ = n;

	Unbind();
}

void VertexArrayObject::SetBuffer(int index, GLenum target, size_t size, const void* data, GLenum usage) {
	Assert(index >= 0 && index < bufferCount_);

	attributes_[index].size = size;
	attributes_[index].target = target;
	attributes_[index].usage = usage;

	BindBuffer(index);
	glBufferData(target, size, data, usage);
	UnbindBuffer(index);
}

void VertexArrayObject::SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, unsigned offset) {
	BindBuffer(index);
	glEnableVertexAttribArray(location);

	if (!normalized && (type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_INT || type == GL_UNSIGNED_INT || type == GL_SHORT || type == GL_UNSIGNED_SHORT)) {
		glVertexAttribIPointer(location, size, type, stride, (void*)(size_t)offset);
	}
	else {
		glVertexAttribPointer(location, size, type, normalized, stride, (void*)(size_t)offset);
	}

	UnbindBuffer(index);
}

unsigned VertexArrayObject::GetBufferNativePointer(int index) {
	Assert(index >= 0 && index < bufferCount_);
	return vbos_[index];
}

void VertexArrayObject::UpdateBuffer(int index, int offset, size_t size, const void* data) {
	Assert(index >= 0 && index < bufferCount_);
	GLuint vbo = vbos_[index];
	const VBOAttribute& attr = attributes_[index];

	BindBuffer(index);
	
	glBufferData(attr.target, attr.size, nullptr, attr.usage); 
	glBufferSubData(attr.target, offset, size, data);
	UnbindBuffer(index);
}

void VertexArrayObject::Destroy() {
	if (bufferCount_ == 0) {
		return;
	}

	glDeleteVertexArrays(1, &vao_);
	vao_ = 0;

	glDeleteBuffers(bufferCount_, vbos_);
	vbos_ = nullptr;

	Memory::ReleaseArray(vbos_);
	vbos_ = nullptr;

	Memory::ReleaseArray(attributes_);
	attributes_ = nullptr;

	bufferCount_ = 0;
}

void VertexArrayObject::Bind() {
	AssertX(vao_ != 0, "invalid vao");
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&oldVao_);
	glBindVertexArray(vao_);
}

void VertexArrayObject::Unbind() {
	glBindVertexArray(oldVao_);
	oldVao_ = 0;
}

void VertexArrayObject::BindBuffer(int index) {
	GLenum pname = GetBindingName(attributes_[index].target);
	glGetIntegerv(pname, (GLint*)&oldBuffer_);

	glBindBuffer(attributes_[index].target, vbos_[index]);
}

void VertexArrayObject::UnbindBuffer(int index) {
	glBindBuffer(attributes_[index].target, oldBuffer_);
	oldBuffer_ = 0;
}

GLenum VertexArrayObject::GetBindingName(GLenum target) {
	if (target == GL_ARRAY_BUFFER) { return GL_ARRAY_BUFFER_BINDING; }
	if (target == GL_ELEMENT_ARRAY_BUFFER) { return GL_ELEMENT_ARRAY_BUFFER_BINDING; }
	AssertX(false, "undefined target binding name");
	return 0;
}
