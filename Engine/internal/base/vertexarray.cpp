#include "buffer.h"
#include "context.h"
#include "vertexarray.h"
#include "debug/debug.h"
#include "memory/refptr.h"

VertexArray::VertexArray(Context* context) : context_(context), vao_(0), oldVao_(0) {
}

VertexArray::~VertexArray() {
	DestroyVertexBuffers();
	context_->DeleteVertexArrays(1, &vao_);
}

void VertexArray::Initialize() {
	if (vao_ == 0) {
		context_->GenVertexArrays(1, &vao_);
	}
}

void VertexArray::CreateVertexBuffers(size_t n) {
	DestroyVertexBuffers();

	Bind();
	vbos_.resize(n, context_);
	Unbind();
}

void VertexArray::SetBuffer(uint index, uint target, size_t size, const void* data, uint usage) {
	vbos_[index].Create(target, size, data, usage);
}

void VertexArray::SetVertexDataSource(int index, int location, int size, uint type, bool normalized, int stride, uint offset, int divisor) {
	vbos_[index].Bind();
	context_->EnableVertexAttribArray(location);

	if (!normalized && IsIPointer(type)) {
		context_->VertexAttribIPointer(location, size, type, stride, (void*)(size_t)offset);
	}
	else {
		context_->VertexAttribPointer(location, size, type, normalized, stride, (void*)(size_t)offset);
	}

	if (divisor > 0) {
		context_->VertexAttribDivisor(location, divisor);
	}

	vbos_[index].Unbind();
}

void* VertexArray::MapBuffer(int index) {
	return vbos_[index].Map();
}

void VertexArray::UnmapBuffer(int index) {
	vbos_[index].Unmap();
}

size_t VertexArray::GetBufferSize(int index) {
	return vbos_[index].GetSize();
}

uint VertexArray::GetBufferNativePointer(uint index) const {
	return vbos_[index].GetNativePointer();
}

void VertexArray::UpdateBuffer(uint index, int offset, size_t size, const void* data) {
	vbos_[index].Update(offset, size, data);
}

void VertexArray::DestroyVertexBuffers() {
	for (int i = 0; i < vbos_.size(); ++i) {
		vbos_[i].Destroy();
	}

	vbos_.clear();
}

void VertexArray::Bind() {
	int old = 0;
	context_->GetIntegerv(GL_VERTEX_ARRAY_BINDING, (int*)&old);
	if (old == vao_) {
		Debug::LogError("vertex array %d already bound.", vao_);
		return;
	}

	oldVao_ = old;
	context_->BindVertexArray(vao_);
}

void VertexArray::Unbind() {
	context_->BindVertexArray(oldVao_);
	oldVao_ = 0;
}

void VertexArray::BindBuffer(uint index) {
	vbos_[index].Bind();
}

void VertexArray::UnbindBuffer(uint index) {
	vbos_[index].Unbind();
}

bool VertexArray::IsIPointer(uint type) {
	return (type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_INT || type == GL_UNSIGNED_INT || type == GL_SHORT || type == GL_UNSIGNED_SHORT);
}
