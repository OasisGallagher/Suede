﻿#pragma once
#include <vector>

#include "shader.h"
#include "../api/gl.h"

class Buffer;
class UniformBuffer {
public:
	UniformBuffer();
	~UniformBuffer();

public:
	bool Create(const std::string& name, uint size);
	
	void AttachBuffer(Shader shader);
	void AttachSubBuffer(Shader shader, uint offset, uint size);
	void UpdateBuffer(const void* data, uint offset, uint size);

	const std::string& GetName() const { return name_; }

private:
	void Destroy();
	void AttachProgram(uint program);
	void Initialize(const std::string& name, uint size);

private:
	uint size_;

	Buffer* ubo_;

	GLuint binding_;
	std::string name_;

private:
	static uint bindingPoint_;
	static uint maxBindingPoints_;
};
