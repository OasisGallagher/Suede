#pragma once
#include <vector>
#include <wrappers/gl.h>

#include "shader.h"
#include "internal/containers/ptrmap.h"
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

inline const ShaderDescription& GetShaderDescription(ShaderType shaderType) {
	static ShaderDescription descriptions[] = {
		GL_VERTEX_SHADER, "VertexShader", "vertex",
		GL_TESS_CONTROL_SHADER, "TessellationControlShader", "tess_control",
		GL_TESS_EVALUATION_SHADER, "TessellationEvaluationShader", "tess_evaluation",
		GL_GEOMETRY_SHADER, "GeometryShader", "geometry",
		GL_FRAGMENT_SHADER, "FragmentShader", "fragment"
	};

	return descriptions[shaderType];
}

class ShaderInternal : public IShader, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Shader)

public:
	

public:
	ShaderInternal();
	~ShaderInternal();

public:
	virtual bool Load(const std::string& path);
	virtual uint GetNativePointer() { return program_; }

	virtual bool SetProperty(const std::string& name, const void* data);
	virtual void GetProperties(std::vector<ShaderProperty>& properties);
private:

	struct Uniform {
		ShaderPropertyType type;
		GLuint size;
		GLuint location;
	};

	typedef PtrMap<std::string, Uniform> UniformContainer;

private:
	bool Link();
	bool LoadSource(ShaderType shaderType, const char* source);
	bool GetErrorMessage(GLuint shaderObj, std::string& answer);
	void ClearIntermediateShaders();

	bool IsSampler(int type);

	void UpdateVertexAttributes();
	void UpdateFragmentAttributes();

	void AddAllUniforms();
	void AddUniform(const char* name, GLenum type, GLuint location, GLint size);
	void SetUniform(Uniform* uniform, const void* data);

private:
	GLuint program_;
	GLuint shaderObjs_[ShaderTypeCount];

	int maxTextureUnits_;
	int textureUnitCount_;
	UniformContainer uniforms_;

	std::string path_;

	static ShaderDescription descriptions_[ShaderTypeCount];
};
