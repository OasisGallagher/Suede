#pragma once
#include "types.h"

class Context;
class Buffer {
public:
	Buffer(Context* context);
	~Buffer();

public:
	void Create(uint target, size_t size, const void* data, uint usage);
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
		char* data;
		size_t size;
		uint target;
		uint usage;
	};

private:
	uint GetBindingName(uint target);

private:
	Context* context_;
	uint old_, buffer_;
	Attribute attribute_;
};
