#pragma once

#include "renderer.h"
#include "internal/containers/variant.h"

class Language;
class SyntaxTree;
class SyntaxNode;

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

struct Semantics {
	struct Property {
		std::string name;
		Variant defaultValue;
	};

	struct Tag {
		std::string key;
		std::string value;
	};

	struct RenderState {
		enum {
			RenderStateParameterCount = 3,
		};

		RenderStateType type;
		int parameters[RenderStateParameterCount];
	};

	struct Pass {
		std::string source;
		std::vector<RenderState> renderStates;
	};

	struct SubShader {
		std::vector<Tag> tags;
		std::vector<Pass> passes;
	};

	std::vector<Property> properties;
	std::vector<SubShader> subShaders;
};

class ShaderLanguage {
public:
	ShaderLanguage();
	~ShaderLanguage();

public:
	bool Parse(const std::string& path, SyntaxTree& tree);

private:
	Language* lang_;
};

class GLSLParser {
public:
	void SetRootPath(const std::string& path) { path_ = path; }
	bool Parse(std::string* sources, const std::string& lines, const std::string& defines);

private:
	void Clear();
	void AddConstants();
	bool Preprocess(const std::string& line);
	bool ReadShaderSource(const std::string& lines);
	bool PreprocessInclude(const std::string& parameter);
	ShaderStage ParseShaderStage(const std::string& tag);
	std::string FormatDefines(const std::string& defines);
	bool PreprocessShaderStage(const std::string& parameter);
	void CalculateDefinesPermutations(std::vector<std::string>& anwser);
	bool CompileShaderSource(const std::string& lines, const std::string& defines);

private:
	ShaderStage type_;
	std::string path_;
	std::string* answer_;
	std::string globals_;
	std::string source_;
};

class ShaderParser {
public:
	bool Parse(Semantics& semantics, const std::string& path, const std::string& defines);

private:
	bool ParseSemantics(SyntaxTree& tree, Semantics& semantices);

	void ReadInt(SyntaxNode* node, Semantics::Property& property);
	void ReadVec3(SyntaxNode* node, Semantics::Property& property);
	void ReadTex2(SyntaxNode* node, Semantics::Property& property);
	void ReadMat3(SyntaxNode* node, Semantics::Property& property);
	void ReadMat4(SyntaxNode* node, Semantics::Property& property);
	void ReadFloat(SyntaxNode* node, Semantics::Property& property);
	void ReadInteger3(SyntaxNode* node, Semantics::Property& property);
	void ReadInteger3(glm::ivec3& value, SyntaxNode* node);

	void ReadProperty(SyntaxNode* node, Semantics::Property& property);
	void ReadProperties(SyntaxNode* node, std::vector<Semantics::Property>& properties);
	void ReadPropertyBlock(SyntaxNode* node, std::vector<Semantics::Property>& properties);

	void ReadTag(SyntaxNode* node, Semantics::Tag& tag);
	void ReadTags(SyntaxNode* node, std::vector<Semantics::Tag>& tags);
	void ReadTagBlock(SyntaxNode* node, std::vector<Semantics::Tag>& tags);

	void ReadRenderState(SyntaxNode* node, Semantics::RenderState& state);
	void ReadRenderStates(SyntaxNode* node, std::vector<Semantics::RenderState>& states);

	void ReadPass(SyntaxNode* node, Semantics::Pass& pass);
	void ReadPasses(SyntaxNode* node, std::vector<Semantics::Pass>& passes);

	void ReadCode(SyntaxNode* node, std::string& source);

	void ReadSubShaderBlock(SyntaxNode* node, Semantics::Semantics::SubShader& subShader);
	void ReadSubShaderBlocks(SyntaxNode* node, std::vector<Semantics::Semantics::SubShader>& subShaders);

	template <class Cont>
	typename Cont::reference Allocate(Cont& cont);

private:
	GLSLParser parser_;
	ShaderLanguage language_;
};

template<class Cont>
inline typename Cont::reference ShaderParser::Allocate(Cont& cont) {
	cont.push_back(Cont::value_type());
	return cont.back();
}
