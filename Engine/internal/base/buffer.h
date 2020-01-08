#pragma once
#include <memory>

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

	size_t GetSize() const { return shadowDataSize_; }
	uint GetNativePointer() const;

private:
	uint GetBindingName(uint target);

private:
	Context* context_;
	uint old_ = 0, buffer_ = 0;

	uint usage_ = 0;
	uint target_ = 0;

	std::unique_ptr<uchar[]> shadowData_;
	size_t shadowDataSize_ = 0;
};
