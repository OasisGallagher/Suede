#pragma once
#include <vector>

#include "gl.h"
#include "enginedefines.h"

class Buffer;
class VertexArray {
public:
	VertexArray();
	~VertexArray();

public:
	void Initialize();

	void CreateVertexBuffers(size_t n);
	void DestroyVertexBuffers();

	size_t GetVBOCount() { return vboCount_; }

	void SetBuffer(uint index, GLenum target, size_t size, const void* data, GLenum usage);

	template <class Container>
	void SetBuffer(uint index, GLenum target, const Container& cont, GLenum usage) {
		SetBuffer(index, target, cont.size() * sizeof(Container::value_type), &cont[0], usage);
	}
	
	void SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, uint offset, int divisor = 0);

	void* MapBuffer(int index);
	void UnmapBuffer(int index);
	size_t GetBufferSize(int index);

	uint GetNativePointer() const { return vao_; }
	uint GetBufferNativePointer(uint index) const;
	void UpdateBuffer(uint index, int offset, size_t size, const void* ptr);

	void Bind();
	void Unbind();

	void BindBuffer(uint index);
	void UnbindBuffer(uint index);

private:
	bool IsIPointer(GLenum type);

private:
	GLuint vao_;
	GLuint oldVao_;

	Buffer* vbos_;
	int vboCount_;
};
