#pragma once
#include <vector>

#include <wrappers/gl.h>
#include "enginedefines.h"

class VAO {
public:
	VAO();
	~VAO();

public:
	void CreateVBOs(size_t n);
	void DestroyVBOs();

	size_t GetVBOCount() { return vboCount_; }

	void SetBuffer(uint index, GLenum target, size_t size, const void* data, GLenum usage);

	template <class Container>
	void SetBuffer(uint index, GLenum target, const Container& cont, GLenum usage, int divisor = 0) {
		SetBuffer(index, target, cont.size() * sizeof(Container::value_type), &cont[0], usage);
	}
	
	void SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, uint offset, int divisor = 0);

	void BindBuffer(int index);
	void UnbindBuffer(int index);

	void* MapBuffer(int index);
	void UnmapBuffer(int index);
	size_t GetBufferSize(int index);

	uint GetBufferNativePointer(uint index);
	void UpdateBuffer(uint index, int offset, size_t size, const void* ptr);

	void Bind();
	void Unbind();

private:
	bool IsIPointer(GLenum type);
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

typedef std::shared_ptr<VAO> VAOPointer;
