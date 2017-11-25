#pragma once
#include "defines.h"
#include <gl/glew.h>

class VertexArrayObject {
public:
	VertexArrayObject();
	~VertexArrayObject();

public:
	void CreateVBOs(size_t n);
	void DestroyVBOs();

	size_t GetVBOCount() { return vboCount_; }

	void SetBuffer(int index, GLenum target, size_t size, const void* data, GLenum usage);

	template <class Container>
	void SetBuffer(int index, GLenum target, const Container& cont, GLenum usage, int divisor = 0) {
		SetBuffer(index, target, cont.size() * sizeof(Container::value_type), &cont[0], usage);
	}
	
	void SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, uint offset, int divisor = 0);

	void BindBuffer(int index);
	void UnbindBuffer(int index);

	uint GetBufferNativePointer(int index);
	void UpdateBuffer(int index, int offset, size_t size, const void* ptr);

	void Bind();
	void Unbind();

private:
	GLenum GetBindingName(GLenum target);

private:
	struct VBOAttribute {
		size_t size;
		GLenum target;
		GLenum usage;
	};

private:
	GLuint vao_;
	GLuint oldVao_;

	GLuint* vbos_;
	GLuint oldBuffer_;
	VBOAttribute* attributes_;

	int vboCount_;
};
