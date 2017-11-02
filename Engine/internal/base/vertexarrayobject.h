#pragma once
#include <gl/glew.h>
class VertexArrayObject {
public:
	VertexArrayObject();
	~VertexArrayObject();

public:
	void Create(size_t n);
	void SetBuffer(int index, GLenum target, size_t size, const void* data, GLenum usage);

	template <class Container>
	void SetBuffer(int index, GLenum target, const Container& cont, GLenum usage) {
		SetBuffer(index, target, cont.size() * sizeof(cont[0]), &cont[0], usage);
	}
	
	void SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, unsigned offset);

	void BindBuffer(int index);
	void UnbindBuffer(int index);

	unsigned GetBufferNativePointer(int index);

	void Destroy();

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

	int bufferCount_;
};
