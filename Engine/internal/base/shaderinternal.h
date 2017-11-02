#pragma once
#include <vector>
#include <gl/glew.h>

#include "shader.h"
#include "internal/base/objectinternal.h"

enum ShaderType {
	ShaderTypeVertex,
	ShaderTypeTessellationControl,
	ShaderTypeTessellationEvaluation,
	ShaderTypeGeometry,
	ShaderTypeFragment,
	ShaderTypeCount,
};

struct ShaderDescription {
	GLenum glShaderType;
	const char* name;
	const char* tag;
};

class ShaderInternal : public IShader, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Shader)

public:
	static const ShaderDescription& Description(ShaderType shaderType) {
		return descriptions_[shaderType];
	}

public:
	ShaderInternal();
	~ShaderInternal();

public:
	virtual bool Load(const std::string& path);
	virtual bool Link();
	virtual unsigned GetNativePointer() { return program_; }

private:
	bool LoadSource(ShaderType shaderType, const char* source);
	bool GetErrorMessage(GLuint shaderObj, std::string& answer);
	void ClearIntermediateShaders();

private:
	GLuint program_;
	GLuint shaderObjs_[ShaderTypeCount];

	std::string shaderPath_;

	static ShaderDescription descriptions_[ShaderTypeCount];
};
