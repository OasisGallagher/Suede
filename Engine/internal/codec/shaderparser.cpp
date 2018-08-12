#include "glef.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "shaderparser.h"
#include "os/filesystem.h"
#include "internal/base/renderdefines.h"

bool GLSLParser::Parse(std::string sources[ShaderStageCount], const std::string& path, const std::string& source, uint ln, const std::string& customDefines) {
	Clear();
	path_ = path;
	answer_ = sources;
	return CompileShaderSource(source, ln, customDefines);
}

void GLSLParser::Clear() {
	type_ = ShaderStageCount;
	source_.clear();

	version_.clear();
	defines_.clear();

	for (int i = 0; i < ShaderStageCount; ++i) {
		ranges_[i].clear();
	}

	includes_.clear();

	answer_ = nullptr;
}

bool GLSLParser::CompileShaderSource(const std::string& source, uint ln, const std::string& customDefines) {
	version_ = "#version " GLSL_VERSION "\n";
	includes_.insert(path_);

	AddDefines(customDefines);
	ReadShaderSource(source, ln);

	if (type_ == ShaderStageCount) {
		Debug::LogError("invalid shader file");
		return false;
	}

	SetShaderStageCode();
	AddShaderSourceRange();

	return true;
}

void GLSLParser::AddDefines(const std::string& customDefines) {
	defines_ += "#define _C_MAX_BONE_COUNT " + std::to_string(MAX_BONE_COUNT) + "\n";
	defines_ += "#define _C_SSAO_KERNAL_SIZE " + std::to_string(SSAO_KERNAL_SIZE) + "\n";
	defines_ += FormatDefines(customDefines);
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

bool GLSLParser::ReadShaderSource(const std::string& source, uint ln) {
	const char* start = source.c_str();
	for (std::string line; String::SplitLine(start, line); ++ln) {
		const char* ptr = String::TrimStart(line.c_str());

		if (type_ != ShaderStageCount) {
			++srcCurrent_;
		}

		if (*ptr == '#' && !Preprocess(ptr, ln)) {
			return false;
		}

		if (type_ == ShaderStageCount && *ptr != 0) {
			Debug::LogError("%d: invalid shader stage.", ln);
			return false;
		}

		if (*ptr != '#' && type_ != ShaderStageCount) {
			source_ += line + '\n';
		}
	}

	return true;
}

bool GLSLParser::PreprocessShaderStage(const std::string& parameter, uint ln) {
	ShaderStage newType = ParseShaderStage(parameter);

	if (newType != type_ && type_ != ShaderStageCount) {
		if (!answer_[type_].empty()) {
			Debug::LogError("%s already exists.", GetShaderDescription(type_).name);
			return false;
		}

		SetShaderStageCode();
		AddShaderSourceRange();
	}

	startln_ = ln + 1;
	srcStart_ = 1;
	// TODO: for defines ...
	srcCurrent_ = 4;

	type_ = newType;

	includes_.clear();

	return true;
}

bool GLSLParser::PreprocessInclude(const std::string& parameter, uint ln) {
	std::string source;

	// skip \" and \".
	std::string path = parameter.substr(1, parameter.length() - 2);
	if (!includes_.insert(path).second) {
		return true;
	}

	if (!FileSystem::ReadAllText(Resources::instance()->GetShaderDirectory() + path, source)) {
		return false;
	}

	path_ = path;
	//source_ += "#line 1\n";
	if (!ReadShaderSource(source, 1)) {
		return false;
	}

	AddShaderSourceRange();
	//source_ += String::Format("#line %d\n", ln + 1);
	return true;
}

bool GLSLParser::Preprocess(const std::string& line, uint ln) {
	size_t pos = line.find(' ');
	std::string cmd = line.substr(1, pos - 1);
	std::string parameter;
	if (pos != std::string::npos) {
		parameter = String::Trim(line.substr(pos));
	}

	if (cmd == GLSL_TAG_STAGE) {
		return PreprocessShaderStage(parameter, ln);
	}

	if (type_ == ShaderStageCount && !line.empty()) {
		Debug::LogError("%d: invalid shader stage.", ln);
		return false;
	}

	if (cmd == GLSL_TAG_INCLUDE) {
		AddShaderSourceRange();
		std::string old = path_;
		int oldSrcStart = srcStart_;
		bool status = PreprocessInclude(parameter, ln);
		
		path_ = old;
		//srcStart_ = srcCurrent_ = oldSrcStart;
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

#include <fstream>
void GLSLParser::SetShaderStageCode() {
	std::string nameDefine = std::string("#define ") + GetShaderDescription(type_).shaderNameDefine + "\n";
	answer_[type_] =
		version_		// #version ...
		//+ String::Format("#line %d\n", startln_)
		+ nameDefine	// #define _VERTEX_SHADER ...
		+ defines_		// #define _C_MAX_BONE_COUNT...
		+ source_;		// GLSL source code.

	std::ofstream ofs(FileSystem::GetFileNameWithoutExtension(path_) + "_" + GetShaderDescription(type_).tag + ".txt");
	ofs << answer_[type_];
	ofs.close();

	source_.clear();
}

void GLSLParser::AddShaderSourceRange() {
	ShaderSourceRanges* ptr = nullptr;
	ShaderSourceRanges::Range range{ srcStart_, srcCurrent_, startln_ };
	for (ShaderSourceRanges& r : ranges_[type_]) {
		if (r.file == path_) {
			ptr = &r;
			break;
		}
	}

	if (ptr != nullptr) {
		// merge range if necessary.
		if (!ptr->ranges.empty() && ptr->ranges.back().last == range.first) {
			ptr->ranges.back().last = range.last;
		}
		else {
			ptr->ranges.push_back(range);
		}
	}
	else {
		ShaderSourceRanges r;
		r.file = path_;
		r.ranges.push_back(range);
		ranges_[type_].push_back(r);
	}

	srcStart_ = srcCurrent_;
}

bool ShaderParser::Parse(Semantics& semantics, const std::string& path, const std::string& customDefines) {
	SyntaxTree tree;
	return GLEF::instance()->Parse((Resources::instance()->GetShaderDirectory() + path).c_str(), tree)
		&& ParseSemantics(tree, semantics);
}

bool ShaderParser::ParseSemantics(SyntaxTree& tree, Semantics& semantics) {
	SyntaxNode* root = tree.GetRoot();
	ReadPropertyBlock(root->GetChildAt(0), semantics.properties);

	SyntaxNode* c1 = root->GetChildAt(1);
	ReadSubShaderBlocks(c1, semantics.subShaders);

	return true;
}

void ShaderParser::ReadProperties(SyntaxNode* node, std::vector<Property*>& properties) {
	ReadTree(node, "Properties", &ShaderParser::ReadProperty, properties);
}

void ShaderParser::ReadTags(SyntaxNode* node, std::vector<Semantics::Tag>& tags) {
	ReadTreeRef(node, "Tags", &ShaderParser::ReadTag, tags);
}

void ShaderParser::ReadSubShaderBlocks(SyntaxNode* node, std::vector<Semantics::SubShader>& subShaders) {
	ReadTreeRef(node, "SubShaders", &ShaderParser::ReadSubShaderBlock, subShaders);
}

void ShaderParser::ReadPasses(SyntaxNode* node, std::vector<Semantics::Pass>& passes) {
	ReadTreeRef(node, "Passes", &ShaderParser::ReadPass, passes);
	if (passes.size() > GLSL_MAX_PASSES) {
		Debug::LogError("pass count must be less equal to %d.", GLSL_MAX_PASSES);
		passes.erase(passes.begin() + GLSL_MAX_PASSES, passes.end());
	}
}

void ShaderParser::ReadRenderStates(SyntaxNode* node, std::vector<Semantics::RenderState>& states) {
	ReadTreeRef(node, "RenderStates", &ShaderParser::ReadRenderState, states);
}

void ShaderParser::ReadInt(SyntaxNode* node, Property* property) {
	property->value.SetInt(String::ToInteger(node->GetChildAt(1)->ToString()));
}

void ShaderParser::ReadSingle(SyntaxNode* node, Property* property) {
	property->value.SetFloat(String::ToFloat(node->GetChildAt(1)->ToString()));
}

void ShaderParser::ReadSingle2(glm::vec2& value, SyntaxNode* node) {
	ReadSingles(node, (float*)&value, 2);
}

void ShaderParser::ReadSingle3(glm::vec3& value, SyntaxNode* node) {
	ReadSingles(node, (float*)&value, 3);
}

void ShaderParser::ReadSingle4(glm::vec4& value, SyntaxNode* node) {
	ReadSingles(node, (float*)&value, 4);
}

void ShaderParser::ReadSingles(SyntaxNode* node, float* ptr, int count) {
	SyntaxNode* c1 = node->GetChildAt(1);
	if (c1 == nullptr) { return; }
	for (int i = 0; i < count && c1->GetChildAt(i) != nullptr; ++i) {
		*ptr++ = String::ToFloat(c1->GetChildAt(i)->ToString());
	}
}

void ShaderParser::ReadInteger(SyntaxNode* node, Property* property) {
	property->value.SetInt(String::ToInteger(node->GetChildAt(1)->ToString()));
}

void ShaderParser::ReadInteger3(glm::ivec3& value, SyntaxNode* node) {
	SyntaxNode* c1 = node->GetChildAt(1);
	if (c1 == nullptr) { return; }
	
	int* ptr = &value.x;
	for (int i = 0; i < 3 && c1->GetChildAt(i) != nullptr; ++i) {
		*ptr++ = String::ToInteger(c1->GetChildAt(i)->ToString());
	}
}

void ShaderParser::ReadVec3(SyntaxNode* node, Property* property) {
	glm::vec3 value;
	ReadSingle3(value, node);
	property->value.SetVector3(value);
}

void ShaderParser::ReadVec4(SyntaxNode* node, Property* property) {
	glm::vec4 value;
	ReadSingle4(value, node);
	property->value.SetVector4(value);
}

void ShaderParser::ReadColor3(SyntaxNode * node, Property * property) {
	glm::vec3 value;
	ReadSingle3(value, node);
	property->value.SetColor3(value);
}

void ShaderParser::ReadColor4(SyntaxNode * node, Property * property) {
	glm::vec4 value;
	ReadSingle4(value, node);
	property->value.SetColor4(value);
}

void ShaderParser::ReadTex2(SyntaxNode* node, Property* property) {
	glm::ivec3 value;
	ReadInteger3(value, node);

	uchar bytes[] = { uchar(value.x & 0xFF), uchar(value.y & 0xFF), uchar(value.z & 0xFF) };
	Texture2D texture = NewTexture2D();
	texture->Load(TextureFormatRgb, bytes, ColorStreamFormatRgb, 1, 1, 4);
	property->value.SetTexture(texture);
}

void ShaderParser::ReadMat3(SyntaxNode* node, Property* property) {
	property->value.SetMatrix3(glm::mat3(0));
}

void ShaderParser::ReadMat4(SyntaxNode* node, Property* property) {
	property->value.SetMatrix4(glm::mat4(0));
}

void ShaderParser::ReadProperty(SyntaxNode* node, Property* property) {
	const std::string& ns = node->ToString();
	std::string name = node->GetChildAt(0)->ToString();
	if (String::StartsWith(name, VARIABLE_PREFIX)) {
		Debug::LogError("property starts with %s is reserved.", VARIABLE_PREFIX);
		return;
	}

	property->name = node->GetChildAt(0)->ToString();

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
	else if (ns == "Color3") {
		ReadColor3(node, property);
	}
	else if (ns == "Color4") {
		ReadColor4(node, property);
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

void ShaderParser::ReadPropertyBlock(SyntaxNode* node, std::vector<Property*>& properties) {
	if (node == nullptr) { return; }

	SyntaxNode* c0 = node->GetChildAt(0);
	if (c0 != nullptr) {
		ReadProperties(c0, properties);
	}
}

void ShaderParser::ReadTag(SyntaxNode* node, Semantics::Tag& tag) {
	tag.key = node->ToString();
	tag.value = node->GetChildAt(0)->ToString();
}

void ShaderParser::ReadTagBlock(SyntaxNode* node, std::vector<Semantics::Tag>& tags) {
	SyntaxNode* c0 = node->GetChildAt(0);
	if (c0 != nullptr) {
		ReadTags(c0, tags);
	}
}

void ShaderParser::ReadRenderState(SyntaxNode* node, Semantics::RenderState& state) {
	state.type = node->GetChildAt(0)->ToString();
	for (uint i = 0; i < Semantics::RenderState::ParameterCount; ++i) {
		SyntaxNode* child = node->GetChildAt(i + 1);
		if (child == nullptr) {
			break;
		}

		state.parameters[i] = child->ToString();
	}
}

void ShaderParser::ReadCode(SyntaxNode* node, std::string& source, uint& lineno) {
	source = node->ToString();
	lineno = node->GetCodeLineNumber();
}

void ShaderParser::ReadPass(SyntaxNode* node, Semantics::Pass& pass) {
	SyntaxNode* c0 = node->GetChildAt(0);
	if (c0 != nullptr) {
		pass.name = c0->ToString();
	}

	SyntaxNode* c1 = node->GetChildAt(1);
	pass.enabled = (c1 == nullptr) || String::ToBool(c1->ToString());

	SyntaxNode* c2 = node->GetChildAt(2);
	if (c2 != nullptr) {
		ReadRenderStates(c2, pass.renderStates);
	}

	SyntaxNode* c3 = node->GetChildAt(3);
	if (c3 != nullptr) {
		ReadCode(c3, pass.source, pass.lineno);
	}
}

void ShaderParser::ReadSubShaderBlock(SyntaxNode* node, Semantics::SubShader& subShader) {
	if (node->GetChildAt(0) != nullptr) {
		ReadTagBlock(node->GetChildAt(0), subShader.tags);
	}

	ReadPasses(node->GetChildAt(1), subShader.passes);
}
