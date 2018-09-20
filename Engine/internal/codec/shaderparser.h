#pragma once

#include <set>
#include "variant.h"
#include "renderer.h"
#include "../api/gl.h"
#include "memory/memory.h"

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
	const char* shaderNameDefine;
};

inline const ShaderDescription& GetShaderDescription(ShaderStage stage) {
	static ShaderDescription descriptions[] = {
		GL_VERTEX_SHADER, "VertexShader", "vertex", "_VERTEX_SHADER",
		GL_TESS_CONTROL_SHADER, "TessellationControlShader", "tess_control", "_TESS_CONTROL_SHADER",
		GL_TESS_EVALUATION_SHADER, "TessellationEvaluationShader", "tess_evaluation", "_TESS_EVALUATION_SHADER",
		GL_GEOMETRY_SHADER, "GeometryShader", "geometry", "_GEOMETRY_SHADER",
		GL_FRAGMENT_SHADER, "FragmentShader", "fragment", "_FRAGMENT_SHADER",
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

		uint lineno;
		std::string name;
		std::string source;
		std::vector<RenderState> renderStates;
	};

	struct SubShader {
		std::vector<Tag> tags;
		std::vector<Pass> passes;
	};

	std::vector<Property*> properties;
	std::vector<SubShader> subShaders;
};

class GLSLParser {
	struct Block {
		// first line in expanded file.
		int first;

		// one pass last line in expanded file.
		int last;

		// map line number in expanded file to shader file by adding this value.
		int offset;
	};

	struct Page {
		// line number of #stage in shader file, or negative line number of first line expanded in included file.
		int tagln;

		// file path.
		std::string file;

		// blocks file splited into.
		std::vector<Block> blocks;
	};

public:
	bool Parse(std::string sources[ShaderStageCount], const std::string& path, const std::string& source, uint ln, const std::string& defines);
	std::string TranslateErrorMessage(ShaderStage stage, const std::string& message);

private:
	void Clear();
	void SetCurrentShaderStageCode();
	
	void AddCurrentBlock();
	void AddBlock(Page& container, Block& block);

	void AddPage(const std::string file, int ln);

	std::string TranslateFileNameAndLineNumber(ShaderStage stage, std::string &msgline);

	bool FindFileNameAndLineNumber(ShaderStage stage, std::string& file, int &ln);
	const char* ParseLineNumberAndMessageBody(const std::string& msgline, int& ln);

	bool Preprocess(const std::string& line);
	void FormatDefines(const std::string& customDefines);

	bool ReadEmptySource();
	bool ReadShaderSource(const std::string& source);

	bool PreprocessInclude(const std::string& parameter);
	ShaderStage ParseShaderStage(const std::string& tag);
	int AddCustomDefines(const std::string& customDefines);
	bool PreprocessShaderStage(const std::string& parameter);
	void CalculateDefinesPermutations(std::vector<std::string>& anwser);
	bool CompileShaderSource(const std::string& source, uint ln, const std::string& customDefines);

private:
	struct {
		// line number in shader file.
		int original;

		// line number in expanded file.
		int expanded;

		// record start of block.
		int start;
	} ln_;

	ShaderStage type_;

	std::string file_;
	std::string currentFile_;

	std::string* answer_;
	
	int ndefines_;
	std::string defines_;

	std::string source_;
	std::set<std::string> includes_;
	std::vector<Page> pages_[ShaderStageCount];
};

class ShaderParser {
public:
	bool Parse(Semantics& semantics, const std::string& path, const std::string& customDefines);

private:
	bool ParseSemantics(SyntaxTree& tree, Semantics& semantices);

	void ReadProperty(SyntaxNode* node, Property* property);
	void ReadProperties(SyntaxNode* node, std::vector<Property*>& properties);
	void ReadPropertyBlock(SyntaxNode* node, std::vector<Property*>& properties);

	void ReadIntProperty(SyntaxNode* node, Property* property);
	void ReadVec3Property(SyntaxNode* node, Property* property);
	void ReadVec4Property(SyntaxNode* node, Property* property);
	void ReadColorProperty(SyntaxNode* node, Property* property);
	void ReadTexture2DProperty(SyntaxNode* node, Property* property);
	void ReadMat3Property(SyntaxNode* node, Property* property);
	void ReadMat4Property(SyntaxNode* node, Property* property);
	void ReadFloatProperty(SyntaxNode* node, Property* property);
	void ReadRangedInt(SyntaxNode* node, Property* property);
	void ReadRangedFloat(SyntaxNode* node, Property* property);

	int ReadInt3(glm::ivec3& value, SyntaxNode* node);
	void ReadString(std::string& value, SyntaxNode* node);
	int ReadVec2(glm::vec2& value, SyntaxNode* node);
	int ReadVec3(glm::vec3& value, SyntaxNode* node);
	int ReadVec4(glm::vec4& value, SyntaxNode* node);
	int ReadFloats(SyntaxNode* node, float* ptr, int count);

	void ReadTag(SyntaxNode* node, Semantics::Tag& tag);
	void ReadTags(SyntaxNode* node, std::vector<Semantics::Tag>& tags);
	void ReadTagBlock(SyntaxNode* node, std::vector<Semantics::Tag>& tags);

	void ReadRenderState(SyntaxNode* node, Semantics::RenderState& state);
	void ReadRenderStates(SyntaxNode* node, std::vector<Semantics::RenderState>& states);

	void ReadPass(SyntaxNode* node, Semantics::Pass& pass);
	void ReadPasses(SyntaxNode* node, std::vector<Semantics::Pass>& passes);

	void ReadCode(SyntaxNode* node, std::string& source, uint& lineno);

	void ReadSubShaderBlock(SyntaxNode* node, Semantics::Semantics::SubShader& subShader);
	void ReadSubShaderBlocks(SyntaxNode* node, std::vector<Semantics::Semantics::SubShader>& subShaders);

	template <class T>
	T* Append(std::vector<T*>& cont);

	template <class T>
	T& AppendRef(std::vector<T>& cont);

	template <class T, class Reader>
	void ReadTree(SyntaxNode* node, const char* plurality, Reader reader, std::vector<T*>& cont);

	template <class T, class Reader>
	void ReadTreeRef(SyntaxNode* node, const char* plurality, Reader reader, std::vector<T>& cont);
};

template<class T>
inline T* ShaderParser::Append(std::vector<T*>& cont) {
	cont.push_back(MEMORY_NEW(T));
	return cont.back();
}

template<class T>
inline T& ShaderParser::AppendRef(std::vector<T>& cont) {
	cont.push_back(T());
	return cont.back();
}

template <class T, class Reader>
void ShaderParser::ReadTree(SyntaxNode* node, const char* plurality, Reader reader, std::vector<T*>& cont) {
	if (node->ToString() == plurality) {
		for (int i = 0; i < node->GetChildCount(); ++i) {
			(this->*reader)(node->GetChildAt(i), Append(cont));
		}
	}
	else {
		(this->*reader)(node, Append(cont));
	}
}

template <class T, class Reader>
void ShaderParser::ReadTreeRef(SyntaxNode* node, const char* plurality, Reader reader, std::vector<T>& cont) {
	if (node->ToString() == plurality) {
		for (int i = 0; i < node->GetChildCount(); ++i) {
			(this->*reader)(node->GetChildAt(i), AppendRef(cont));
		}
	}
	else {
		(this->*reader)(node, AppendRef(cont));
	}
}
