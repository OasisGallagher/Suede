#pragma once
#include <vector>
#include <wrappers/gl.h>

#include "shader.h"
#include "renderstate.h"
#include "containers/ptrmap.h"
#include "internal/file/shaderparser.h"
#include "internal/base/objectinternal.h"

class Pass {
public:
	Pass();
	~Pass();

public:
	bool Initialize(const Semantics::Pass& pass, const std::string& directory);
	bool SetProperty(const std::string& name, const void* data);

	void Bind();
	void Unbind();

private:
	struct Uniform {
		VariantType type;
		GLuint size;
		GLuint location;
	};

	typedef PtrMap<std::string, Uniform> UniformContainer;

	void InitializeRenderStates(const std::vector<Semantics::RenderState>& states);
	RenderState* CreateRenderState(const Semantics::RenderState& state);
	RenderState* AllocateRenderState(const Semantics::RenderState &state);

	bool ParseRenderStateParameters(int* answer, const std::string* parameters);
	int RenderStateParameterToInteger(const std::string& parameter);

	bool Link();
	bool LoadSource(ShaderStage stage, const char* source);
	bool GetErrorMessage(GLuint shaderObj, std::string& answer);
	void ClearIntermediateShaders();

	void BindRenderStates();
	void UnbindRenderStates();

	bool IsSampler(int type);

	void UpdateVertexAttributes();
	void UpdateFragmentAttributes();

	void AddAllUniforms();
	void AddUniform(const char* name, GLenum type, GLuint location, GLint size);
	void SetUniform(Uniform* uniform, const void* data);
	void SetUniform(GLuint location, VariantType type, uint size, const void* data);

private:
	GLuint program_;
	GLuint oldProgram_;
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

	void Bind(uint pass);
	void Unbind();

	void EnablePass(uint pass);
	void DisablePass(uint pass);
	bool IsPassEnabled(uint pass) const;

	Pass* GetPass(uint pass);
	uint GetPassCount() const { return passCount_; }

private:
	void InitializeTags(const std::vector<Semantics::Tag>& tags);
	void InitializeTag(const Semantics::Tag& tag, uint i);
	uint ParseExpression(TagKey key, const std::string& expression);

private:
	Pass* passes_;
	uint passCount_;
	uint passEnabled_;
	uint currentPass_;

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

	virtual void Bind(uint subShader, uint pass);
	virtual void Unbind();

	virtual void EnablePass(uint subShader, uint pass);
	virtual void DisablePass(uint subShader, uint pass);
	virtual bool IsPassEnabled(uint subShader, uint pass) const;

	virtual uint GetPassCount(uint subShader) const { return subShaders_[subShader].GetPassCount(); }
	virtual void GetProperties(std::vector<Property>& properties);
	virtual bool SetProperty(uint subShader, uint pass, const std::string& name, const void* data);

private:
	void ParseProperties(std::vector<Property>& properties);
	void ParseSubShaders(std::vector<Semantics::SubShader>& subShaders, const std::string& path);

private:
	std::string path_;
	
	std::vector<Property> properties_;

	SubShader* subShaders_;
	uint subShaderCount_;
	uint currentSubShader_;
};
