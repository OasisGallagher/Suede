#pragma once
#include <vector>
#include <wrappers/gl.h>

#include "shader.h"
#include "internal/containers/ptrmap.h"
#include "internal/base/objectinternal.h"

enum ShaderStage {
	ShaderStageVertex,
	ShaderStageTessellationControl,
	ShaderStageTessellationEvaluation,
	ShaderStageGeometry,
	ShaderStageFragment,
	ShaderStageCount,
};

struct ShaderDescription {
	GLenum glShaderStage;
	const char* name;
	const char* tag;
};

inline const ShaderDescription& GetShaderDescription(ShaderStage stage) {
	static ShaderDescription descriptions[] = {
		GL_VERTEX_SHADER, "VertexShader", "vertex",
		GL_TESS_CONTROL_SHADER, "TessellationControlShader", "tess_control",
		GL_TESS_EVALUATION_SHADER, "TessellationEvaluationShader", "tess_evaluation",
		GL_GEOMETRY_SHADER, "GeometryShader", "geometry",
		GL_FRAGMENT_SHADER, "FragmentShader", "fragment"
	};

	return descriptions[stage];
}

class ShaderPass {
public:
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

class ShaderInternal : public IShader, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Shader)

public:
	ShaderInternal();
	~ShaderInternal();

public:
	virtual bool Load(const std::string& path);
	virtual uint GetNativePointer() { return program_; }

	virtual void Bind(uint pass);
	virtual void Unbind();

	virtual uint GetPassCount() { return passes_.size(); }

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
	bool LoadSource(ShaderStage stage, const char* source);
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
	GLuint shaderObjs_[ShaderStageCount];

	int maxTextureUnits_;
	int textureUnitCount_;
	UniformContainer uniforms_;

	std::string path_;
	uint currentPass_;
	std::vector<ShaderPass*> passes_;

	static ShaderDescription descriptions_[ShaderStageCount];
};
