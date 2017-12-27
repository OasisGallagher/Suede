#include "math2.h"
#include "variables.h"
#include "textloader.h"
#include "tools/path.h"
#include "shadercompiler.h"
#include "tools/string.h"
#include "internal/base/glsldefines.h"

#include "lr_grammar.h"

#include "language.h"
#include "syntax_tree.h"

ShaderLanguage::ShaderLanguage() {
	lang_ = MEMORY_CREATE(Language);
	lang_->Setup(grammar, compiler);
}

ShaderLanguage::~ShaderLanguage() {
	MEMORY_RELEASE(lang_);
}

bool ShaderLanguage::Parse(const std::string& code, SyntaxTree& tree) {
	if (lang_->Parse(&tree, code)) {
		return true;
	}

	// error message.
	return false;
}

bool ShaderCompiler::Compile(const std::string& path, const std::string& defines, Semantics& semantices) {
	std::string code;
	if (!TextLoader::Load(Path::GetResourceRootDirectory() + path, code)) {
		return false;
	}

	SyntaxTree tree;
	if (!language_->Parse(code, &tree)) {
		return false;
	}

	if (!ParseSemantics(tree, semantices)) {
		return false;
	}

	/*
	Clear();

	answer_ = answer;
	path_ = path;
	return CompileShaderSource(lines, defines);
	*/
}

void ShaderCompiler::Clear() {
	type_ = ShaderStageCount;
	source_.clear();
	globals_.clear();
	answer_ = nullptr;
}

bool ShaderCompiler::CompileShaderSource(const std::vector<std::string>& lines, const std::string& defines) {
	globals_ = "#version " GLSL_VERSION "\n";

	AddConstants();

	globals_ += FormatDefines(defines);
	ReadShaderSource(lines);

	if (type_ == ShaderStageCount) {
		Debug::LogError("invalid shader file");
		return false;
	}

	answer_[type_] = globals_ + source_;

	return true;
}

void ShaderCompiler::AddConstants() {
	globals_ += "#define C_MAX_BONE_COUNT " + std::to_string(C_MAX_BONE_COUNT) + "\n";
}

std::string ShaderCompiler::FormatDefines(const std::string& defines) {
	std::vector<std::string> container;
	String::Split(defines, '|', container);

	std::string ans;
	for (int i = 0; i < container.size(); ++i) {
		ans += "#define " + container[i] + "\n";
	}

	return ans;
}

bool ShaderCompiler::Preprocess(const std::string& line) {
	size_t pos = line.find(' ');
	std::string cmd = line.substr(1, pos - 1);
	std::string parameter;
	if (pos != std::string::npos) {
		parameter = String::Trim(line.substr(pos));
	}

	if (cmd == GLSL_TAG_SHADER) {
		return PreprocessShader(parameter);
	}

	if (cmd == GLSL_TAG_INCLUDE) {
		std::string old = path_;
		bool status = PreprocessInclude(parameter);
		path_ = old;
		return status;
	}

	source_ += line + '\n';
	return true;
}

void ShaderCompiler::CalculateDefinesPermutations(std::vector<std::string>& anwser) {
	std::vector<std::string> defines_;
	int max = 1 << 0; defines_.size();
	for (int i = 0; i < max; ++i) {
		std::string perm;
		const char* sep = "";
		for (int j = 0; j < defines_.size(); ++j) {
			if (((1 << j) & i) != 0) {
				perm += sep;
				sep = "|";
				perm += defines_[j];
			}
		}

		anwser.push_back(perm);
	}
}

ShaderStage ShaderCompiler::ParseShaderStage(const std::string& tag) {
	for (size_t i = 0; i < ShaderStageCount; ++i) {
		if (tag == GetShaderDescription((ShaderStage)i).tag) {
			return (ShaderStage)i;
		}
	}

	Debug::LogError("unkown shader tag %s.", tag.c_str());
	return ShaderStageCount;
}

bool ShaderCompiler::ParseSemantics(SyntaxTree& tree, Semantics& semantices) {
	SyntaxNode* root = tree->GetRoot();
	ReadPropertyBlock(root->GetChild(0), semantices.properties);

	SyntaxNode* c1 = root->GetChild(1);
	if (c1->GetText() == "ShaderBlock") {
		ReadSubShaderBlock(c1, Allocate(semantices.subShaders));
	}
	else {
		ReadSubShaderBlocks(c1, semantices.subShaders);
	}
}

void ShaderCompiler::ReadInt(SyntaxNode* node, ShaderProperty& property) {
	property.defaultValue.SetInt(String::ToInteger(node->GetChild(1)->GetText()));
}

void ShaderCompiler::ReadFloat(SyntaxNode* node, ShaderProperty& property) {
	property.defaultValue.SetInt(String::ToFloat(node->GetChild(1)->GetText()));
}

void ShaderCompiler::ReadNumber3(SyntaxNode* node, ShaderProperty& property) {
	glm::vec3 value;
	float* ptr = &value.x;
	for (int i = 0; i < Math::Min(3, node->GetChildCount()); ++i) {
		*ptr++ = String::ToFloat(node->GetChild(i)->GetText());
	}

	property.defaultValue.SetVector3(value);
}

void ShaderCompiler::ReadVec3(SyntaxNode* node, ShaderProperty& property) {
	if (node->GetChildCount() >= 2) {
		ReadNumber3(node->GetChild(1), property);
	}
}

void ShaderCompiler::ReadTex2(SyntaxNode* node, ShaderProperty& property) {
#error initialize color here.
	Texture2D texture = NewTexture2D();
	texture->Load(&color, ColorFormatRgba, 1, 1);
	property.defaultValue->SetTexture(texture);
}

void ShaderCompiler::ReadMat3(SyntaxNode* node, ShaderProperty& property) {
	property.defaultValue->SetMatrix3(glm::mat3(0));
}

void ShaderCompiler::ReadMat4(SyntaxNode* node, ShaderProperty& property) {
	property.defaultValue->SetMatrix3(glm::mat4(0));
}

void ShaderCompiler::ReadProperty(SyntaxNode* node, ShaderProperty& property) {
	const std::string& ns = node->GetText();
	property.name = node->GetChild(0)->GetText();

	if (ns == "Int") {
		ReadInt(node, property);
	}
	else if (ns == "Float") {
		ReadFloat(node, property);
	}
	else if (ns == "Vec3") {
		ReadVec3(node, property);
	}
	else if (ns == "Tex2") {
		ReadTex2(node, property);
	}
	else if (ns == "Mat3") {
		ReadMat3(node, property);
	}
	else if (ns == "Mat4") {
		ReadMat4(node, property);
	}
	else {
		Debug::LogError("invalid property type %s.", ns.c_str());
	}
}

void ShaderCompiler::ReadProperties(SyntaxNode* node, std::vector<ShaderProperty>& properties) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->GetText() == "Properties") {
		ReadProperties(c0, properties);
		ReadProperty(node->GetChild(1), Allocate(properties));
	}
	else {
		ReadProperty(c0, Allocate(properties));
		if (node->GetChildCount() >= 2) {
			ReadProperty(node->GetChild(1), Allocate(properties));
		}
	}
}

void ShaderCompiler::ReadPropertyBlock(SyntaxNode* node, std::vector<ShaderProperty>& properties) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0 == nullptr) {
		return;
	}

	if (c0->GetText() == "Properties") {
		ReadProperties(c0, properties);
		if (node->GetChildCount() >= 2) {
			ReadProperty(node->GetChild(1), Allocate(properties));
		}
	}
	else {
		ReadProperty(c0, Allocate(properties));
	}
}

void ShaderCompiler::ReadTag(SyntaxNode* node, SubShaderTag& tag) {
#error parse tag key and value.
	Debug::Log(node->GetText() + " " + node->GetChild(0)->GetText());
}

void ShaderCompiler::ReadTags(SyntaxNode* node, std::vector<SubShaderTag>& tags) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->GetText() == "Tags") {
		ReadTags(c0, tags);
		ReadTag(node->GetChild(1), Allocate(tags));
	}
	else {
		ReadTag(c0, Allocate(tags));
		if (node->GetChildCount() >= 2) {
			ReadTag(node->GetChild(1), Allocate(tags));
		}
	}
}

void ShaderCompiler::ReadTagBlock(SyntaxNode* node, std::vector<SubShaderTag>& tags) {
	if (node->GetChild(0) != nullptr) {
		ReadTags(node->GetChild(0), tags);
	}
}

void ShaderCompiler::ReadRenderState(SyntaxNode* node, SubShaderRenderState& state) {
#error subShader render state.
	Debug::Log(node->GetChild(0)->GetText() + " " + node->GetChild(1)->GetText());
}

void ShaderCompiler::ReadRenderStates(SyntaxNode* node, std::vector<SubShaderRenderState>& states) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->GetText() == "RenderStates") {
		ReadRenderStates(c0, states);
		ReadRenderState(node->GetChild(1), Allocate(states));
	}
	else {
		ReadRenderState(c0, Allocate(states));
		if (node->GetChildCount() >= 2) {
			ReadRenderState(node->GetChild(1), Allocate(states));
		}
	}
}

void ShaderCompiler::ReadCode(SyntaxNode* node, std::string& vertex, std::string& fragment) {
	Debug::Log(node->GetText());
}

void ShaderCompiler::ReadPass(SyntaxNode* node, ShaderPass& pass) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0 != nullptr) {
		if (c0->GetText() == "RenderState") {
			ReadRenderState(c0, Allocate(pass.renderStates));
		}
		else {
			ReadRenderStates(c0, pass.renderStates);
		}
	}

	if (node->GetChild(1) != nullptr) {
		ReadCode(node->GetChild(1), pass.vertex, pass.fragment);
	}
}

void ShaderCompiler::ReadPasses(SyntaxNode* node, std::vector<ShaderPass>& passes) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->GetText() == "ShaderPass") {
		ReadPass(c0, Allocate(passes));
		if (node->GetChildCount() >= 2) {
			ReadPass(node->GetChild(1), Allocate(passes));
		}
	}
	else {
		ReadPasses(c0, passes);
		ReadPass(node->GetChild(1), Allocate(passes));
	}
}

void ShaderCompiler::ReadSubShaderBlock(SyntaxNode* node, SubShader& subShader) {
	if (node->GetChild(0) != nullptr) {
		ReadTagBlock(node->GetChild(0), subShader.tags);
	}

	if (node->GetChild(1)->GetText() == "ShaderPass") {
		ReadPass(node->GetChild(1), subShader.passes);
	}
	else {
		ReadPasses(node->GetChild(1), Allocate(subShader.passes));
	}
}

void ShaderCompiler::ReadSubShaderBlocks(SyntaxNode* node, std::vector<SubShader>& subShaders) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->GetText() == "ShaderBlocks") {
		ReadSubShaderBlocks(c0, subShaders);
		ReadSubShaderBlock(node->GetChild(1), Allocate(subShaders));
	}
	else {
		ReadSubShaderBlock(c0, Allocate(subShaders));
		if (node->GetChildCount() >= 2) {
			ReadSubShaderBlock(node->GetChild(1), Allocate(subShaders));
		}
	}
}

bool ShaderCompiler::ReadShaderSource(const std::vector<std::string> &lines) {
	for (size_t i = 0; i < lines.size(); ++i) {
		const std::string& line = lines[i];
		if (line.front() == '#' && !Preprocess(line)) {
			return false;
		}

		if (line.front() != '#') {
			source_ += line + '\n';
		}
	}

	return true;
}

bool ShaderCompiler::PreprocessShader(const std::string& parameter) {
	ShaderStage newType = ParseShaderStage(parameter);

	if (newType != type_) {
		if (type_ == ShaderStageCount) {
			globals_ += source_;
		}
		else {
			if (!answer_[type_].empty()) {
				Debug::LogError("%s already exists.", GetShaderDescription(type_).name);
				return false;
			}

			source_ = globals_ + source_;
			answer_[type_] = source_;
		}

		source_.clear();
		type_ = newType;
	}

	return true;
}

bool ShaderCompiler::PreprocessInclude(const std::string& parameter) {
	std::vector<std::string> lines;
	std::string path = parameter.substr(1, parameter.length() - 2);
	if (!TextLoader::Load(Path::GetResourceRootDirectory() + path, lines)) {
		return false;
	}

	path_ = path;

	return ReadShaderSource(lines);
}
