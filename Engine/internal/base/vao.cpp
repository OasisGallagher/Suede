#include "vao.h"
#include "debug/debug.h"
#include "memory/memory.h"

VAO::VAO() 
	: vao_(0), oldVao_(0), vbos_(nullptr), attributes_(nullptr), oldBuffer_(0)
	, vboCount_(0) {
	GL::GenVertexArrays(1, &vao_);
}

VAO::~VAO() {
	DestroyVBOs();
	GL::DeleteVertexArrays(1, &vao_);
}

void VAO::CreateVBOs(size_t n) {
	DestroyVBOs();

	Bind();

	vbos_ = MEMORY_CREATE_ARRAY(GLuint, n);
	GL::GenBuffers(n, vbos_);
	attributes_ = MEMORY_CREATE_ARRAY(VBOAttribute, n);

	vboCount_ = n;

	Unbind();
}

void VAO::SetBuffer(uint index, GLenum target, size_t size, const void* data, GLenum usage) {
	if (index >= vboCount_) {
		Debug::LogError("index out of range");
		return;
	}

	attributes_[index].size = size;
	attributes_[index].target = target;
	attributes_[index].usage = usage;

	BindBuffer(index);
	GL::BufferData(target, size, data, usage);
	UnbindBuffer(index);
}

void VAO::SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, uint offset, int divisor) {
	BindBuffer(index);
	GL::EnableVertexAttribArray(location);

	if (!normalized && IsIPointer(type)) {
		GL::VertexAttribIPointer(location, size, type, stride, (void*)(size_t)offset);
	}
	else {
		GL::VertexAttribPointer(location, size, type, normalized, stride, (void*)(size_t)offset);
	}

	if (divisor > 0) {
		GL::VertexAttribDivisor(location, divisor);
	}

	UnbindBuffer(index);
}

void* VAO::MapBuffer(int index) {
	if (index >= vboCount_) {
		Debug::LogError("index out of range");
		return nullptr;
	}

	return GL::MapBuffer(attributes_[index].target, GL_READ_ONLY);
}

void VAO::UnmapBuffer(int index) {
	if (index >= vboCount_) {
		Debug::LogError("index out of range");
		return;
	}
	
	GL::UnmapBuffer(attributes_[index].target);
}

size_t VAO::GetBufferSize(int index) {
	if (index >= vboCount_) {
		Debug::LogError("index out of range");
		return 0;
	}

	return attributes_[index].size;
}

uint VAO::GetBufferNativePointer(uint index) const {
	if (index >= vboCount_) {
		Debug::LogError("index out of range");
		return 0;
	}

	return vbos_[index];
}

void VAO::UpdateBuffer(uint index, int offset, size_t size, const void* data) {
	if (index >= vboCount_) {
		Debug::LogError("index out of range");
		return;
	}

	GLuint vbo = vbos_[index];
	VBOAttribute& attr = attributes_[index];

	BindBuffer(index);

	GL::BufferData(attr.target, attr.size, nullptr, attr.usage); 
	GL::BufferSubData(attr.target, offset, size, data);

	UnbindBuffer(index);
}

void VAO::DestroyVBOs() {
	if (vboCount_ == 0) {
		return;
	}

	GL::DeleteBuffers(vboCount_, vbos_);
	MEMORY_RELEASE_ARRAY(vbos_);
	vbos_ = nullptr;

	MEMORY_RELEASE_ARRAY(attributes_);
	attributes_ = nullptr;

	vboCount_ = 0;
}

void VAO::Bind() {
	GL::GetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&oldVao_);
	GL::BindVertexArray(vao_);
}

void VAO::Unbind() {
	GL::BindVertexArray(oldVao_);
	oldVao_ = 0;
}

void VAO::BindBuffer(int index) {
	GLenum pname = GetBindingName(attributes_[index].target);
	GL::GetIntegerv(pname, (GLint*)&oldBuffer_);

	GL::BindBuffer(attributes_[index].target, vbos_[index]);
}

void VAO::UnbindBuffer(int index) {
	GL::BindBuffer(attributes_[index].target, oldBuffer_);
	oldBuffer_ = 0;
}

GLenum VAO::GetBindingName(GLenum target) {
	if (target == GL_ARRAY_BUFFER) { return GL_ARRAY_BUFFER_BINDING; }
	if (target == GL_ELEMENT_ARRAY_BUFFER) { return GL_ELEMENT_ARRAY_BUFFER_BINDING; }
	Debug::LogError("undefined target binding name");
	return 0;
}

bool VAO::IsIPointer(GLenum type) {
	return (type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_INT || type == GL_UNSIGNED_INT || type == GL_SHORT || type == GL_UNSIGNED_SHORT);
}
