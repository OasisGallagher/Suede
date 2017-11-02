#pragma once
#include <gl/glew.h>

class VertexArrayObject;

class TransformFeedback {
public:
	TransformFeedback();
	~TransformFeedback();

public:
	void Create(size_t n, size_t size, const void* data);
	void Destroy();

	void Bind(int tfbIndex, int vboIndex);
	void Unbind();

	void SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, unsigned offset);

private:
	size_t tfCount_;
	VertexArrayObject* vao_;

	GLuint* tfbs_;
	GLuint oldTfb_;
	int bindingVboIndex_;
};
