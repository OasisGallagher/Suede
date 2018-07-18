#pragma once
#include "api/gl.h"

class Buffer {
public:
	Buffer();
	~Buffer();

public:
	void Create(GLenum target, size_t size, const void* data, GLenum usage);
	void Destroy();

	void Bind();
	void Unbind();

	void* Map();
	void Unmap();
	void Update(int offset, size_t size, const void* data);

	size_t GetSize() const { return attribute_.size; }
	uint GetNativePointer() const { return buffer_; }

private:
	struct Attribute {
		size_t size;
		GLenum target;
		GLenum usage;
	};

private:
	GLenum GetBindingName(GLenum target);

private:
	GLuint old_, buffer_;
	Attribute attribute_;
};
