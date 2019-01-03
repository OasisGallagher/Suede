#pragma once
#include <vector>

#include "shader.h"
#include "../api/gl.h"
#include "renderstate.h"
#include "containers/ptrmap.h"
#include "internal/codec/shaderparser.h"
#include "internal/base/objectinternal.h"

class Pass {
public:
	Pass();
	~Pass();

public:
	bool Initialize(std::vector<Property*>& properties, const Semantics::Pass& pass, const std::string& directory);
	bool SetProperty(const std::string& name, const void* data);

	const std::string& GetName() const { return name_; }

	void Bind();
	void Unbind();
	uint GetNativePointer() const { return program_; }

private:
	struct Uniform {
		VariantType type;
		GLuint size;
		GLuint location;
	};

	typedef ptr_map<std::string, Uniform> UniformContainer;

	void InitializeRenderStates(const std::vector<Semantics::RenderState>& states);
	RenderState* CreateRenderState(const Semantics::RenderState& state);
	RenderState* AllocateRenderState(const Semantics::RenderState &state);

	bool ParseRenderStateParameters(int* answer, const std::string* parameters);
	bool RenderStateParameterToInteger(const std::string& parameter, int& answer);

	bool Link();
	std::string LoadSource(ShaderStage stage, const char* source);
	bool GetErrorMessage(GLuint shaderObj, std::string& answer);
	void ClearIntermediateShaders();

	void BindRenderStates();
	void UnbindRenderStates();

	bool IsSampler(int type);

	void UpdateVertexAttributes();
	void UpdateFragmentAttributes();

	void AddAllUniforms();
	void AddAllUniformProperties(std::vector<Property*>& properties);

	void AddUniform(const char* name, GLenum type, GLuint location, GLint size);
	void AddUniformProperty(std::vector<Property*>& properties, const std::string& name, VariantType type);

	void SetUniform(Uniform* uniform, const void* data);
	void SetUniform(GLuint location, VariantType type, uint size, const void* data);

private:
	GLuint program_;
	GLuint oldProgram_;
	GLuint shaderObjs_[ShaderStageCount];

	std::string name_;
	std::string path_;

	int textureUnitCount_;
	UniformContainer uniforms_;
	RenderState* states_[RenderStateType::size()];
};

class SubShader {
public:
	enum TagKey {
		TagKeyRenderQueue,
		TagKeyCount,
	};

	struct Tag {
		TagKey key;
		uint value;
	};

public:
	SubShader();
	~SubShader();

public:
	bool Initialize(std::vector<ShaderProperty>& properties, const Semantics::SubShader& config, const std::string& path);

	void Bind(uint pass);
	void Unbind();

	void SetRenderQueue(int value) { tags_[TagKeyRenderQueue] = value; }
	int GetRenderQueue() const { return tags_[TagKeyRenderQueue]; }

	/**
	* @return initial enabled state of pass `pass`.
	*/
	bool IsPassEnabled(uint pass) const;
	int GetPassIndex(const std::string& name) const;
	
	Pass* GetPass(uint pass);
	const Pass* GetPass(uint pass) const;

	uint GetPassCount() const { return passCount_; }
	uint GetNativePointer(uint pass) const;

private:
	void InitializeTags(const std::vector<Semantics::Tag>& tags);
	void InitializeTag(const Semantics::Tag& tag, uint i);
	uint ParseExpression(TagKey key, const std::string& expression);
	bool CheckPropertyCompatible(ShaderProperty* target, Property* p);
	void AddShaderProperties(std::vector<ShaderProperty>& properties, const std::vector<Property*> container, uint pass);

private:
	Pass* passes_;
	uint passCount_;
	uint passEnabled_;
	uint currentPass_;

	int tags_[TagKeyCount];
};

class ShaderInternal : public ObjectInternal {
public:
	ShaderInternal();
	~ShaderInternal();

public:
	std::string GetName() const;
	bool Load(IShader* self, const std::string& path);

	void Bind(uint ssi, uint pass);
	void Unbind();

	void SetRenderQueue(uint ssi, int value);
	int GetRenderQueue(uint ssi) const;

	bool IsPassEnabled(uint ssi, uint pass) const;
	int GetPassIndex(uint ssi, const std::string& name) const;
	uint GetNativePointer(uint ssi, uint pass) const;

	uint GetPassCount(uint ssi) const { return subShaders_[ssi].GetPassCount(); }
	uint GetSubShaderCount() const { return subShaderCount_; }

	void GetProperties(std::vector<ShaderProperty>& properties);
	bool SetProperty(uint ssi, uint pass, const std::string& name, const void* data);

private:
	void ReleaseProperties();
	void SetProperties(const std::vector<ShaderProperty>& properties);
	void ParseSemanticProperties(std::vector<ShaderProperty>& properties, const Semantics& semantics);
	void ParseSubShader(std::vector<ShaderProperty>& properties, const std::vector<Semantics::SubShader>& subShaders, const std::string& path);

private:
	std::string path_;
	std::vector<ShaderProperty> properties_;

	SubShader* subShaders_;
	uint subShaderCount_;
	uint currentSubShader_;
};
