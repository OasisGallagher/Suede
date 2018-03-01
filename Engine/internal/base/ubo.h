#pragma once
#include <vector>

#include "shader.h"
#include "wrappers/gl.h"

class UBO {
public:
	static UBO* Create(const std::string& name, uint size);
	static void GetAllUBOs(std::vector<UBO*>& container);

public:
	void Attach(Shader shader);
	void SetBuffer(const void* data, uint offset, uint size);

private:
	UBO();
	~UBO();

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
