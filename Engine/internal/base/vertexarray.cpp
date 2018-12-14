#include "buffer.h"
#include "vertexarray.h"
#include "debug/debug.h"
#include "memory/memory.h"

VertexArray::VertexArray() 
	: vao_(0), oldVao_(0), vbos_(nullptr), vboCount_(0) {
}

VertexArray::~VertexArray() {
	DestroyVertexBuffers();
	GL::DeleteVertexArrays(1, &vao_);
}

void VertexArray::Initialize() {
	if (vao_ == 0) {
		GL::GenVertexArrays(1, &vao_);
	}
}

void VertexArray::CreateVertexBuffers(size_t n) {
	DestroyVertexBuffers();

	Bind();

	vbos_ = MEMORY_NEW_ARRAY(Buffer, n);
	vboCount_ = n;

	Unbind();
}

void VertexArray::SetBuffer(uint index, GLenum target, size_t size, const void* data, GLenum usage) {
	SUEDE_VERIFY_INDEX(index, vboCount_, SUEDE_NOARG);
	vbos_[index].Create(target, size, data, usage);
}

void VertexArray::SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, uint offset, int divisor) {
	vbos_[index].Bind();
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

	vbos_[index].Unbind();
}

void* VertexArray::MapBuffer(int index) {
	SUEDE_VERIFY_INDEX(index, vboCount_, nullptr);
	return vbos_[index].Map();
}

void VertexArray::UnmapBuffer(int index) {
	SUEDE_VERIFY_INDEX(index, vboCount_, SUEDE_NOARG);
	vbos_[index].Unmap();
}

size_t VertexArray::GetBufferSize(int index) {
	SUEDE_VERIFY_INDEX(index, vboCount_, 0);
	return vbos_[index].GetSize();
}

uint VertexArray::GetBufferNativePointer(uint index) const {
	SUEDE_VERIFY_INDEX(index, vboCount_, 0);
	return vbos_[index].GetNativePointer();
}

void VertexArray::UpdateBuffer(uint index, int offset, size_t size, const void* data) {
	SUEDE_VERIFY_INDEX(index, vboCount_, SUEDE_NOARG);
	vbos_[index].Update(offset, size, data);
}

void VertexArray::DestroyVertexBuffers() {
	for (uint i = 0; i < vboCount_; ++i) {
		vbos_[i].Destroy();
	}

	vboCount_ = 0;
	MEMORY_DELETE_ARRAY(vbos_);
}

void VertexArray::Bind() {
	GLint old = 0;
	GL::GetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&old);
	if (old == vao_) {
		Debug::LogError("vertex array %d already bound.", vao_);
		return;
	}

	oldVao_ = old;
	GL::BindVertexArray(vao_);
}

void VertexArray::Unbind() {
	GL::BindVertexArray(oldVao_);
	oldVao_ = 0;
}

void VertexArray::BindBuffer(uint index) {
	SUEDE_VERIFY_INDEX(index, vboCount_, SUEDE_NOARG);
	vbos_[index].Bind();
}

void VertexArray::UnbindBuffer(uint index) {
	SUEDE_VERIFY_INDEX(index, vboCount_, SUEDE_NOARG);
	vbos_[index].Unbind();
}

bool VertexArray::IsIPointer(GLenum type) {
	return (type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_INT || type == GL_UNSIGNED_INT || type == GL_SHORT || type == GL_UNSIGNED_SHORT);
}
