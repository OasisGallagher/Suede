#pragma once
#include <vector>
#include <wrappers/gl.h>

#include "shader.h"
#include "internal/containers/ptrmap.h"
#include "internal/file/shaderparser.h"
#include "internal/base/objectinternal.h"

class RenderState;

class Pass {
public:
	Pass();
	~Pass();

public:
	bool Initialize(const Semantics::Pass& pass, const std::string& path);

private:
	struct Uniform {
		ShaderPropertyType type;
		GLuint size;
		GLuint location;
	};

	typedef PtrMap<std::string, Uniform> UniformContainer;
	void InitializeRenderStates(std::vector<Semantics::RenderState> states);

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

	std::string path_;

	int maxTextureUnits_;
	int textureUnitCount_;
	UniformContainer uniforms_;

	RenderState* states_[RenderStateCount];
};

class SubShader {
public:
	enum TagKey {
		TagKeyQueue,
	};

	struct Tag {
		TagKey key;
		uint value;
	};

public:
	SubShader();
	~SubShader();

public:
	bool Initialize(const Semantics::SubShader& config, const std::string& path);

private:
	void InitializeTags(const std::vector<Semantics::Tag>& tags);

private:
	Pass* passes_;
	uint passCount_;

	Tag* tags_;
	uint tagCount_;
};

class ShaderInternal : public IShader, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Shader)

public:
	ShaderInternal();
	~ShaderInternal();

public:
	virtual bool Load(const std::string& path);

	virtual void Bind(uint pass);
	virtual void Unbind();

	virtual uint GetPassCount() { return 0; }

	virtual bool SetProperty(const std::string& name, const void* data);
	virtual void GetProperties(std::vector<ShaderProperty>& properties);

private:
	void ParseProperties(std::vector<Semantics::Property>& properties);
	void ParseSubShaders(std::vector<Semantics::SubShader>& subShaders, const std::string& path);

private:
	

	std::string path_;
	uint currentPass_;

	std::vector<Semantics::Property> properties_;

	SubShader* subShaders_;
	uint subShaderCount_;
};
