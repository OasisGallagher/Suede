#include "glef.h"
#include "variables.h"
#include "tools/file.h"
#include "tools/path.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "shaderparser.h"
#include "tools/string.h"
#include "memory/memory.h"
#include "internal/base/glsldefines.h"

bool GLSLParser::Parse(std::string* sources, const std::string& path, const std::string& source, const std::string& defines) {
	Clear();
	path_ = path;
	answer_ = sources;
	return CompileShaderSource(source, defines);
}

void GLSLParser::Clear() {
	type_ = ShaderStageCount;
	source_.clear();
	globals_.clear();
	answer_ = nullptr;
}

bool GLSLParser::CompileShaderSource(const std::string& source, const std::string& defines) {
	globals_ = "#version " GLSL_VERSION "\n";

	AddConstants();

	globals_ += FormatDefines(defines);
	ReadShaderSource(source);

	if (type_ == ShaderStageCount) {
		Debug::LogError("invalid shader file");
		return false;
	}

	answer_[type_] = globals_ + source_;

	return true;
}

void GLSLParser::AddConstants() {
	globals_ += "#define C_MAX_BONE_COUNT " + std::to_string(C_MAX_BONE_COUNT) + "\n";
}

std::string GLSLParser::FormatDefines(const std::string& defines) {
	std::vector<std::string> container;
	String::Split(container, defines, '|');

	std::string ans;
	for (int i = 0; i < container.size(); ++i) {
		ans += "#define " + container[i] + "\n";
	}

	return ans;
}

bool GLSLParser::ReadShaderSource(const std::string &source) {
	const char* start = source.c_str();
	std::string line;
	for (uint ln = 1; String::SplitLine(start, line); ) {
		const char* ptr = String::TrimStart(line.c_str());
		if (*ptr == '#' && !Preprocess(ptr)) {
			return false;
		}

		if (*ptr != '#') {
			source_ += line + '\n';
		}
	}

	return true;
}

bool GLSLParser::PreprocessShaderStage(const std::string& parameter) {
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

bool GLSLParser::PreprocessInclude(const std::string& parameter) {
	std::string source;
	std::string path = parameter.substr(1, parameter.length() - 2);
	if (!File::Load(Path::GetResourceRootDirectory() + path, source)) {
		return false;
	}

	path_ = path;

	return ReadShaderSource(source);
}

bool GLSLParser::Preprocess(const std::string& line) {
	size_t pos = line.find(' ');
	std::string cmd = line.substr(1, pos - 1);
	std::string parameter;
	if (pos != std::string::npos) {
		parameter = String::Trim(line.substr(pos));
	}

	if (cmd == GLSL_TAG_STAGE) {
		return PreprocessShaderStage(parameter);
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

void GLSLParser::CalculateDefinesPermutations(std::vector<std::string>& anwser) {
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

ShaderStage GLSLParser::ParseShaderStage(const std::string& tag) {
	for (size_t i = 0; i < ShaderStageCount; ++i) {
		if (tag == GetShaderDescription((ShaderStage)i).tag) {
			return (ShaderStage)i;
		}
	}

	Debug::LogError("unkown shader tag %s.", tag.c_str());
	return ShaderStageCount;
}

bool ShaderParser::Parse(Semantics& semantics, const std::string& path, const std::string& defines) {
	SyntaxTree tree;
	if (!GLEF::Parse(path.c_str(), tree)) {
		return false;
	}

	if (!ParseSemantics(tree, semantics)) {
		return false;
	}

	return true;
}

bool ShaderParser::ParseSemantics(SyntaxTree& tree, Semantics& semantics) {
	SyntaxNode* root = tree.GetRoot();
	ReadPropertyBlock(root->GetChild(0), semantics.properties);

	SyntaxNode* c1 = root->GetChild(1);
	if (c1->ToString() == "SubShader") {
		ReadSubShaderBlock(c1, Append(semantics.subShaders));
	}
	else {
		ReadSubShaderBlocks(c1, semantics.subShaders);
	}

	return true;
}

void ShaderParser::ReadInt(SyntaxNode* node, Property& property) {
	property.value.SetInt(String::ToInteger(node->GetChild(1)->ToString()));
}

void ShaderParser::ReadSingle(SyntaxNode* node, Property& property) {
	property.value.SetFloat(String::ToFloat(node->GetChild(1)->ToString()));
}

void ShaderParser::ReadSingle3(SyntaxNode* node, Property& property) {
	glm::vec3 value;
	ReadSingle3(value, node);
	property.value.SetVector3(value);
}

void ShaderParser::ReadSingle3(glm::vec3& value, SyntaxNode* node) {
	SyntaxNode* c1 = node->GetChild(1);
	if (c1 != nullptr) {
		float* ptr = &value.x;
		for (int i = 0; i < 3 && c1->GetChild(i) != nullptr; ++i) {
			*ptr++ = String::ToFloat(c1->GetChild(i)->ToString());
		}
	}
}

void ShaderParser::ReadSingle4(SyntaxNode* node, Property& property) {
	glm::vec4 value;
	ReadSingle4(value, node);
	property.value.SetVector4(value);
}

void ShaderParser::ReadSingle4(glm::vec4& value, SyntaxNode* node) {
	SyntaxNode* c1 = node->GetChild(1);
	if (c1 != nullptr) {
		float* ptr = &value.x;
		for (int i = 0; i < 4 && c1->GetChild(i) != nullptr; ++i) {
			*ptr++ = String::ToFloat(c1->GetChild(i)->ToString());
		}
	}
}

void ShaderParser::ReadInteger(SyntaxNode* node, Property& property) {
	property.value.SetInt(String::ToInteger(node->GetChild(1)->ToString()));
}

void ShaderParser::ReadInteger3(SyntaxNode* node, Property& property) {
	glm::ivec3 value;
	ReadInteger3(value, node);
	property.value.SetIVector3(value);
}

void ShaderParser::ReadInteger3(glm::ivec3& value, SyntaxNode* node) {
	SyntaxNode* c1 = node->GetChild(1);
	if (c1 != nullptr) {
		int* ptr = &value.x;
		for (int i = 0; i < 3 && c1->GetChild(i) != nullptr; ++i) {
			*ptr++ = String::ToInteger(c1->GetChild(i)->ToString());
		}
	}
}

void ShaderParser::ReadVec3(SyntaxNode* node, Property& property) {
	if (node->GetChildCount() >= 2) {
		ReadSingle3(node, property);
	}
}

void ShaderParser::ReadVec4(SyntaxNode* node, Property& property) {
	if (node->GetChildCount() >= 2) {
		ReadSingle4(node, property);
	}
}

void ShaderParser::ReadTex2(SyntaxNode* node, Property& property) {
	glm::ivec3 value;
	ReadInteger3(value, node);

	uchar bytes[] = { uchar(value.x & 0xFF), uchar(value.y & 0xFF), uchar(value.z & 0xFF) };
	Texture2D texture = NewTexture2D();
	texture->Load(bytes, ColorFormatRgb, 1, 1);
	property.value.SetTexture(texture);
}

void ShaderParser::ReadMat3(SyntaxNode* node, Property& property) {
	property.value.SetMatrix3(glm::mat3(0));
}

void ShaderParser::ReadMat4(SyntaxNode* node, Property& property) {
	property.value.SetMatrix4(glm::mat4(0));
}

void ShaderParser::ReadProperty(SyntaxNode* node, Property& property) {
	const std::string& ns = node->ToString();
	property.name = node->GetChild(0)->ToString();

	if (ns == "Int") {
		ReadInt(node, property);
	}
	else if (ns == "Single") {
		ReadSingle(node, property);
	}
	else if (ns == "Vec3") {
		ReadVec3(node, property);
	}
	else if (ns == "Vec4") {
		ReadVec4(node, property);
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

void ShaderParser::ReadProperties(SyntaxNode* node, std::vector<Property>& properties) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->ToString() == "Properties") {
		ReadProperties(c0, properties);
		ReadProperty(node->GetChild(1), Append(properties));
	}
	else {
		ReadProperty(c0, Append(properties));
		if (node->GetChildCount() >= 2) {
			ReadProperty(node->GetChild(1), Append(properties));
		}
	}
}

void ShaderParser::ReadPropertyBlock(SyntaxNode* node, std::vector<Property>& properties) {
	if (node == nullptr) { return; }

	SyntaxNode* c0 = node->GetChild(0);
	if (c0 == nullptr) { return; }

	if (c0->ToString() == "Properties") {
		ReadProperties(c0, properties);
		if (node->GetChildCount() >= 2) {
			ReadProperty(node->GetChild(1), Append(properties));
		}
	}
	else {
		ReadProperty(c0, Append(properties));
	}
}

void ShaderParser::ReadTag(SyntaxNode* node, Semantics::Tag& tag) {
	tag.key = node->ToString();
	tag.value = node->GetChild(0)->ToString();
}

void ShaderParser::ReadTags(SyntaxNode* node, std::vector<Semantics::Tag>& tags) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->ToString() == "Tags") {
		ReadTags(c0, tags);
		ReadTag(node->GetChild(1), Append(tags));
	}
	else {
		ReadTag(c0, Append(tags));
		if (node->GetChildCount() >= 2) {
			ReadTag(node->GetChild(1), Append(tags));
		}
	}
}

void ShaderParser::ReadTagBlock(SyntaxNode* node, std::vector<Semantics::Tag>& tags) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0 == nullptr) { return; }

	if (c0->ToString() == "Tags") {
		ReadTags(c0, tags);
	}
	else {
		ReadTag(c0, Append(tags));
	}
}

void ShaderParser::ReadRenderState(SyntaxNode* node, Semantics::RenderState& state) {
	state.type = node->GetChild(0)->ToString();
	for (uint i = 0; i < Semantics::RenderState::ParameterCount; ++i) {
		SyntaxNode* child = node->GetChild(i + 1);
		if (child == nullptr) {
			break;
		}

		state.parameters[i] = child->ToString();
	}
}

void ShaderParser::ReadRenderStates(SyntaxNode* node, std::vector<Semantics::RenderState>& states) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->ToString() == "RenderStates") {
		ReadRenderStates(c0, states);
		ReadRenderState(node->GetChild(1), Append(states));
	}
	else {
		ReadRenderState(c0, Append(states));
		if (node->GetChildCount() >= 2) {
			ReadRenderState(node->GetChild(1), Append(states));
		}
	}
}

void ShaderParser::ReadCode(SyntaxNode* node, std::string& source) {
	source = node->ToString();
}

void ShaderParser::ReadPass(SyntaxNode* node, Semantics::Pass& pass) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0 != nullptr) {
		if (c0->ToString() == "RenderState") {
			ReadRenderState(c0, Append(pass.renderStates));
		}
		else {
			ReadRenderStates(c0, pass.renderStates);
		}
	}

	if (node->GetChild(1) != nullptr) {
		ReadCode(node->GetChild(1), pass.source);
	}
}

void ShaderParser::ReadPasses(SyntaxNode* node, std::vector<Semantics::Pass>& passes) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->ToString() == "Pass") {
		ReadPass(c0, Append(passes));
		if (node->GetChildCount() >= 2) {
			ReadPass(node->GetChild(1), Append(passes));
		}
	}
	else {
		ReadPasses(c0, passes);
		ReadPass(node->GetChild(1), Append(passes));
	}
}

void ShaderParser::ReadSubShaderBlock(SyntaxNode* node, Semantics::SubShader& subShader) {
	if (node->GetChild(0) != nullptr) {
		ReadTagBlock(node->GetChild(0), subShader.tags);
	}

	if (node->GetChild(1)->ToString() == "Pass") {
		ReadPass(node->GetChild(1), Append(subShader.passes));
	}
	else {
		ReadPasses(node->GetChild(1), subShader.passes);
	}
}

void ShaderParser::ReadSubShaderBlocks(SyntaxNode* node, std::vector<Semantics::SubShader>& subShaders) {
	SyntaxNode* c0 = node->GetChild(0);
	if (c0->ToString() == "SubShaders") {
		ReadSubShaderBlocks(c0, subShaders);
		ReadSubShaderBlock(node->GetChild(1), Append(subShaders));
	}
	else {
		ReadSubShaderBlock(c0, Append(subShaders));
		if (node->GetChildCount() >= 2) {
			ReadSubShaderBlock(node->GetChild(1), Append(subShaders));
		}
	}
}
