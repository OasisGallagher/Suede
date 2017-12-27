#pragma once

#include "renderer.h"
#include "internal/containers/variant.h"
#include "internal/base/shaderinternal.h"

class Language;
class SyntaxTree;
class SyntaxNode;

struct ShaderProperty {
	std::string name;
	Variant defaultValue;
};

enum ShaderTagKey {
	ShaderTagKeyQueue,
};

struct SubShaderTag {
	ShaderTagKey key;
	uint value;
};

struct SubShaderRenderState {
	enum {
		RenderStateParameterCount = 3,
	};

	RenderStateType type;
	int parameters[RenderStateParameterCount];
};

struct ShaderPass {
	std::string vertex;
	std::string fragment;
	std::vector<SubShaderRenderState> renderStates;
};

struct SubShader {
	std::vector<SubShaderTag> tags;
	std::vector<ShaderPass> passes;
};

struct Semantics {
	std::vector<ShaderProperty> properties;
	std::vector<SubShader> subShaders;
};

class ShaderLanguage {
public:
	ShaderLanguage();
	~ShaderLanguage();

public:
	bool Parse(const std::string& code, SyntaxTree& tree);

private:
	Language* lang_;
};

class ShaderCompiler {
public:
	bool Compile(const std::string& path, const std::string& defines, Semantics& semantics);

private:
	void Clear();
	bool ParseSemantics(SyntaxTree& tree, Semantics& semantices);
	bool ReadShaderSource(const std::vector<std::string> &lines);
	std::string FormatDefines(const std::string& defines);
	bool CompileShaderSource(const std::vector<std::string>& lines, const std::string& defines);

	void AddConstants();

	bool Preprocess(const std::string& line);
	bool PreprocessInclude(const std::string& parameter);
	bool PreprocessShader(const std::string& parameter);

	void CalculateDefinesPermutations(std::vector<std::string>& anwser);

	ShaderStage ParseShaderStage(const std::string& tag);

	void ReadInt(SyntaxNode* node, ShaderProperty& property);
	void ReadVec3(SyntaxNode* node, ShaderProperty& property);
	void ReadTex2(SyntaxNode* node, ShaderProperty& property);
	void ReadMat3(SyntaxNode* node, ShaderProperty& property);
	void ReadMat4(SyntaxNode* node, ShaderProperty& property);
	void ReadFloat(SyntaxNode* node, ShaderProperty& property);
	void ReadNumber3(SyntaxNode* node, ShaderProperty& property);

	void ReadProperty(SyntaxNode* node, ShaderProperty& property);
	void ReadProperties(SyntaxNode* node, std::vector<ShaderProperty>& properties);
	void ReadPropertyBlock(SyntaxNode* node, std::vector<ShaderProperty>& properties);

	void ReadTag(SyntaxNode* node, SubShaderTag& tag);
	void ReadTags(SyntaxNode* node, std::vector<SubShaderTag>& tags);
	void ReadTagBlock(SyntaxNode* node, std::vector<SubShaderTag>& tags);

	void ReadRenderState(SyntaxNode* node, SubShaderRenderState& state);
	void ReadRenderStates(SyntaxNode* node, std::vector<SubShaderRenderState>& states);

	void ReadPass(SyntaxNode* node, ShaderPass& pass);
	void ReadPasses(SyntaxNode* node, std::vector<ShaderPass>& passes);

	void ReadCode(SyntaxNode* node, std::string& vertex, std::string& fragment);

	void ReadSubShaderBlock(SyntaxNode* node, SubShader& subShader);
	void ReadSubShaderBlocks(SyntaxNode* node, std::vector<SubShader>& subShaders);

	template <class Cont>
	Cont::reference Allocate(Cont& cont);

private:
	ShaderStage type_;
	std::string path_;
	std::string* answer_;
	std::string globals_;
	std::string source_;
	ShaderLanguage language_;
};

template<class Cont>
inline Cont::reference ShaderCompiler::Allocate(Cont& cont) {
	cont.push_back(Cont::value_type());
	return cont.back();
}
