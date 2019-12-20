#pragma once
#include <vector>

#include "shader.h"

class Buffer;
class Context;
class UniformBuffer {
public:
	UniformBuffer(Context* context);
	~UniformBuffer();

public:
	bool Create(const std::string& name, uint size);
	
	void AttachBuffer(Shader* shader);
	void AttachSubBuffer(Shader* shader, uint offset, uint size);
	bool UpdateBuffer(const void* data, uint offset, uint size);

	const std::string& GetName() const { return name_; }

private:
	void Destroy();
	void Attach(Shader* shader);
	void AttachProgram(uint program);
	void Initialize(const std::string& name, uint size);

private:
	Context* context_;

	uint size_;

	Buffer* ubo_;

	uint binding_;
	std::string name_;

private:
	static uint bindingPoint_;
	static uint maxBindingPoints_;
};
