#pragma once
#include <vector>

#include "shader.h"
#include "wrappers/gl.h"

class UBO {
public:
	UBO();
	~UBO();

public:
	bool Create(const std::string& name, uint size);
	
	void AttachSharedBuffer(Shader shader);
	void AttachEntityBuffer(Shader shader, uint offset, uint size);

	void SetBuffer(const void* data, uint offset, uint size);

	const std::string& GetName() const { return name_; }

private:
	void Bind();
	void Unbind();
	void Destroy();
	void AttachToProgram(uint program);
	void Initialize(const std::string& name, uint size);

private:
	uint size_;

	GLuint ubo_;
	GLuint oldUbo_;
	GLuint binding_;
	std::string name_;
};
