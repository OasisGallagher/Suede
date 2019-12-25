#pragma once
#include <vector>

#include "defines.h"
#include "containers/dynamicarray.h"

class Buffer;
class Context;
class VertexArray {
public:
	VertexArray(Context* context);
	~VertexArray();

public:
	void Initialize();

	void CreateVertexBuffers(size_t n);
	void DestroyVertexBuffers();

	size_t GetVBOCount() { return vbos_.size(); }

	void SetBuffer(uint index, uint target, size_t size, const void* data, uint usage);

	template <class Container>
	void SetBuffer(uint index, uint target, const Container& cont, uint usage) {
		SetBuffer(index, target, cont.size() * sizeof(Container::value_type), &cont[0], usage);
	}
	
	void SetVertexDataSource(int index, int location, int size, uint type, bool normalized, int stride, uint offset, int divisor = 0);

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
	bool IsIPointer(uint type);
	void Destroy();

private:
	Context* context_;
	uint vao_;
	uint oldVao_;

	dynamic_array<Buffer> vbos_;
};
