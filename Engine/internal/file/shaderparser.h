#pragma once

#include "variant.h"
#include "renderer.h"
#include "wrappers/gl.h"

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
	struct Tag {
		std::string key;
		std::string value;
	};

	struct RenderState {
		enum {
			ParameterCount = 3,
		};

		std::string type;
		std::string parameters[ParameterCount];
	};

	struct Pass {
		bool enabled;
		std::string name;
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

class GLSLParser {
public:
	bool Parse(std::string* sources, const std::string& path, const std::string& source, const std::string& defines);

private:
	void Clear();
	void AddConstants();
	bool Preprocess(const std::string& line);
	bool ReadShaderSource(const std::string& source);
	bool PreprocessInclude(const std::string& parameter);
	ShaderStage ParseShaderStage(const std::string& tag);
	std::string FormatDefines(const std::string& defines);
	bool PreprocessShaderStage(const std::string& parameter);
	void CalculateDefinesPermutations(std::vector<std::string>& anwser);
	bool CompileShaderSource(const std::string& source, const std::string& defines);

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

	void ReadInt(SyntaxNode* node, Property& property);
	
	void ReadVec3(SyntaxNode* node, Property& property);
	void ReadVec4(SyntaxNode* node, Property& property);

	void ReadTex2(SyntaxNode* node, Property& property);
	
	void ReadMat3(SyntaxNode* node, Property& property);
	void ReadMat4(SyntaxNode* node, Property& property);
	
	void ReadSingle(SyntaxNode* node, Property& property);
	void ReadInteger(SyntaxNode* node, Property& property);

	void ReadSingle3(SyntaxNode* node, Property& property);
	void ReadSingle3(glm::vec3& value, SyntaxNode* node);
	
	void ReadSingle4(SyntaxNode* node, Property& property);
	void ReadSingle4(glm::vec4& value, SyntaxNode* node);
	
	void ReadInteger3(SyntaxNode* node, Property& property);
	void ReadInteger3(glm::ivec3& value, SyntaxNode* node);

	void ReadProperty(SyntaxNode* node, Property& property);
	void ReadProperties(SyntaxNode* node, std::vector<Property>& properties);
	void ReadPropertyBlock(SyntaxNode* node, std::vector<Property>& properties);

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
	typename Cont::reference Append(Cont& cont);

	template <class Cont, class Reader>
	void ReadTree(SyntaxNode* node, const char* plurality, Reader reader, Cont& cont);
};

template<class Cont>
inline typename Cont::reference ShaderParser::Append(Cont& cont) {
	cont.push_back(Cont::value_type());
	return cont.back();
}

template <class Cont, class Reader>
void ShaderParser::ReadTree(SyntaxNode* node, const char* plurality, Reader reader, Cont& cont) {
	if (node->ToString() == plurality) {
		for (int i = 0; i < node->GetChildCount(); ++i) {
			(this->*reader)(node->GetChild(i), Append(cont));
		}
	}
	else {
		(this->*reader)(node, Append(cont));
	}
}
