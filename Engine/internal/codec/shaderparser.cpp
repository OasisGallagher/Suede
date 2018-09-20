#include "glef.h"
#include "resources.h"
#include "tools/math2.h"
#include "shaderparser.h"
#include "os/filesystem.h"
#include "builtinproperties.h"
#include "internal/base/renderdefines.h"

bool GLSLParser::Parse(std::string sources[ShaderStageCount], const std::string& path, const std::string& source, uint ln, const std::string& customDefines) {
	Clear();
	FormatDefines(customDefines);

	answer_ = sources;
	file_ = currentFile_ = path + ".shader";
	return CompileShaderSource(source, ln, customDefines);
}

std::string GLSLParser::TranslateErrorMessage(ShaderStage stage, const std::string& message) {
	std::vector<std::string> lines;
	String::Split(lines, message, '\n');

	std::string answer, info;
	for (std::string msgline : lines) {
		if (msgline.empty()) { continue; }
		if (!answer.empty()) { answer += '\n'; }

		answer += TranslateFileNameAndLineNumber(stage, msgline);
	}

	return answer;
}

void GLSLParser::Clear() {
	type_ = ShaderStageCount;
	source_.clear();

	for (int i = 0; i < ShaderStageCount; ++i) {
		pages_[i].clear();
	}

	defines_.clear();
	includes_.clear();

	answer_ = nullptr;
}

bool GLSLParser::CompileShaderSource(const std::string& source, uint ln, const std::string& customDefines) {
	ln_.original = ln;
	ln_.expanded = ndefines_;

	ReadShaderSource(source);

	if (type_ == ShaderStageCount) {
		Debug::LogError("%s: invalid shader file", file_.c_str());
		return false;
	}

	SetCurrentShaderStageCode();
	AddCurrentBlock();

	return true;
}

void GLSLParser::FormatDefines(const std::string& customDefines) {
	defines_ = "#version " GLSL_VERSION "\n";
	defines_ += "#define %s\n";
	defines_ += "#define _C_MAX_BONE_COUNT " + std::to_string(MAX_BONE_COUNT) + "\n";
	defines_ += "#define _C_SSAO_KERNEL_SIZE " + std::to_string(SSAO_KERNEL_SIZE) + "\n";
	ndefines_ = 4 + AddCustomDefines(customDefines);
}

int GLSLParser::AddCustomDefines(const std::string& defines) {
	std::vector<std::string> container;
	String::Split(container, defines, '|');

	std::string ans;
	for (int i = 0; i < container.size(); ++i) {
		defines_ += "#define " + container[i] + "\n";
	}

	return container.size();
}

bool GLSLParser::ReadShaderSource(const std::string& source) {
	if (source.empty()) {
		return ReadEmptySource();
	}

	std::vector<std::string> lines;
	String::Split(lines, source, '\n');
	for (std::string line : lines) {
		const char* ptr = String::TrimStart(line.c_str());

		if (*ptr == '#' && !Preprocess(ptr)) {
			return false;
		}

		if (type_ == ShaderStageCount &&*ptr != 0) {
			Debug::LogError("%s(%d): invalid shader stage.", file_.c_str(), ln_.original);
			return false;
		}
		
		if (currentFile_ == file_) {
			++ln_.original;
		}

		if (*ptr != '#' && type_ != ShaderStageCount) {
			source_ += line + '\n';
			++ln_.expanded;
		}
	}

	return true;
}

bool GLSLParser::ReadEmptySource() {
	source_ += '\n';
	++ln_.expanded;
	return true;
}

bool GLSLParser::PreprocessShaderStage(const std::string& parameter) {
	if (currentFile_ != file_) {
		Debug::LogError("%s(%d): #stage directive is only valid in .shader file", file_.c_str(), ln_.original);
		return false;
	}

	ShaderStage newType = ParseShaderStage(parameter);

	if (newType != type_ && type_ != ShaderStageCount) {
		if (!answer_[type_].empty()) {
			Debug::LogError("%s(%d): %s already exists.", file_.c_str(), ln_.original, GetShaderDescription(type_).name);
			return false;
		}

		SetCurrentShaderStageCode();
		AddCurrentBlock();
	}

	ln_.start = 1;
	ln_.expanded = ndefines_;

	type_ = newType;

	AddPage(currentFile_, ln_.original);

	includes_.clear();

	return true;
}

bool GLSLParser::PreprocessInclude(const std::string& parameter) {
	std::string source;

	// skip \" and \".
	std::string target = parameter.substr(1, parameter.length() - 2);
	if (!includes_.insert(target).second) {
		Debug::LogError("%s(%d): already included %s.", file_.c_str(), ln_.original, target.c_str());
		return false;
	}

	if (!FileSystem::ReadAllText(Resources::instance()->GetShaderDirectory() + target, source)) {
		return false;
	}

	AddPage(target, -(ln_.expanded + 1));

	currentFile_ = target;
	if (!ReadShaderSource(source)) {
		return false;
	}

	AddCurrentBlock();
	return true;
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

	if (type_ == ShaderStageCount && !line.empty()) {
		Debug::LogError("%s(%d): invalid shader stage.", file_.c_str(), ln_.original);
		return false;
	}

	if (cmd == GLSL_TAG_INCLUDE) {
		AddCurrentBlock();

		std::string old = currentFile_;
		bool status = PreprocessInclude(parameter);
		currentFile_ = old;
		return status;
	}

	++ln_.expanded;
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

	Debug::LogError("%s(%d): unkown shader tag %s.", file_.c_str(), ln_.original, tag.c_str());
	return ShaderStageCount;
}

void GLSLParser::SetCurrentShaderStageCode() {
	answer_[type_] =
		String::Format(defines_.c_str(), GetShaderDescription(type_).shaderNameDefine)
		+ source_;		// GLSL source code.

	source_.clear();
}

void GLSLParser::AddCurrentBlock() {
	for (Page& page : pages_[type_]) {
		if (page.file == currentFile_) {
			Block block{ ln_.start, ln_.expanded + 1 };
			AddBlock(page, block);
			break;
		}
	}
}

std::string GLSLParser::TranslateFileNameAndLineNumber(ShaderStage stage, std::string& msgline) {
	int ln = 0;
	const char* body = ParseLineNumberAndMessageBody(msgline, ln);

	std::string file; 
	if (body == nullptr || !FindFileNameAndLineNumber(stage, file, ln)) {
		return msgline;
	}

	return String::Format("%s(%d):\n%s", file.c_str(), ln, body);
}

const char* GLSLParser::ParseLineNumberAndMessageBody(const std::string& msgline, int& ln) {
	ln = 0;
	const char* ptr = msgline.c_str();
	// find file number.
	for (;*ptr != 0 && !isdigit(*ptr); ++ptr) {}

	// skip file number.
	for (;*ptr != 0 && isdigit(*ptr); ++ptr) {}

	// find line number.
	for (;*ptr != 0 && !isdigit(*ptr); ++ptr) {}

	// parse line number.
	for (;*ptr != 0 && isdigit(*ptr); ++ptr) { ln = ln* 10 +*ptr - '0'; }

	// find error message body.
	for (;*ptr != 0 && !isalpha(*ptr); ++ptr) {}

	if (ln == 0) {
		Debug::LogError("failed to translate shader error message: unknown format.");
		return nullptr;
	}

	return ptr;
}

bool GLSLParser::FindFileNameAndLineNumber(ShaderStage stage, std::string& file, int& ln) {
	for (Page& p : pages_[stage]) {
		for (Block& b : p.blocks) {
			if (ln >= b.first && ln < b.last) {
				file = p.file;
				ln = ln + b.offset;
				break;
			}
		}

		if (!file.empty()) {
			break;
		}
	}

	if (file.empty()) {
		Debug::LogError("failed to translate shader error message: logic error.");
		return false;
	}
	
	if (String::EndsWith(file, ".shader") && ln <= ndefines_) {
		Debug::LogError("failed to translate shader error message: internal error.");
		return false;
	}

	return true;
}

void GLSLParser::AddBlock(Page& pages, Block& block) {
	// merge blocks if necessary.
	if (!pages.blocks.empty() && pages.blocks.back().last == block.first) {
		pages.blocks.back().last = block.last;
	}
	else if (!pages.blocks.empty()) {
		Block& prev = pages.blocks.back();
		block.offset = (prev.last + prev.offset) - block.first + 1;
		pages.blocks.push_back(block);
	}
	else {
		if (pages.tagln > 0) {
			block.offset = pages.tagln - ndefines_;
		}
		else {
			block.offset = 1 - (-pages.tagln);
		}

		pages.blocks.push_back(block);
	}

	ln_.start = ln_.expanded + 1;
}

void GLSLParser::AddPage(const std::string file, int ln) {
	Page page;
	page.file = file;
	page.tagln = ln;
	pages_[type_].push_back(page);
}

bool ShaderParser::Parse(Semantics& semantics, const std::string& file, const std::string& customDefines) {
	SyntaxTree tree;
	return GLEF::instance()->Parse((Resources::instance()->GetShaderDirectory() + file).c_str(), tree)
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

int ShaderParser::ReadInt3(glm::ivec3& value, SyntaxNode* node) {
	SyntaxNode* c1 = node->GetChildAt(1);
	if (c1 == nullptr) { return 0; }

	int* ptr = &value.x, i = 0;
	for (; i < 3 && c1->GetChildAt(i) != nullptr; ++i) {
		*ptr++ = String::ToInteger(c1->GetChildAt(i)->ToString());
	}

	return i;
}

void ShaderParser::ReadString(std::string& value, SyntaxNode* node) {
	value = node->GetChildAt(1)->ToString();
}

void ShaderParser::ReadFloatProperty(SyntaxNode* node, Property* property) {
	property->value.SetFloat(String::ToFloat(node->GetChildAt(1)->ToString()));
}

void ShaderParser::ReadRangedInt(SyntaxNode* node, Property* property) {
	glm::ivec3 value(0, INT_MIN, INT_MAX);
	ReadInt3(value, node);
	property->value.SetRangedInt(*(iranged*)&value);
}

void ShaderParser::ReadRangedFloat(SyntaxNode* node, Property* property) {
	glm::vec3 value(0, -FLT_MAX, FLT_MAX);
	ReadVec3(value, node);
	property->value.SetRangedFloat(*(franged*)&value);
}

int ShaderParser::ReadVec2(glm::vec2& value, SyntaxNode* node) {
	return ReadFloats(node, (float*)&value, 2);
}

int ShaderParser::ReadVec3(glm::vec3& value, SyntaxNode* node) {
	return ReadFloats(node, (float*)&value, 3);
}

int ShaderParser::ReadVec4(glm::vec4& value, SyntaxNode* node) {
	return ReadFloats(node, (float*)&value, 4);
}

int ShaderParser::ReadFloats(SyntaxNode* node, float* ptr, int count) {
	SyntaxNode* c1 = node->GetChildAt(1);
	if (c1 == nullptr) { return 0; }

	int i = 0;
	for (; i < count && c1->GetChildAt(i) != nullptr; ++i) {
		*ptr++ = String::ToFloat(c1->GetChildAt(i)->ToString());
	}

	return i;
}

void ShaderParser::ReadIntProperty(SyntaxNode* node, Property* property) {
	property->value.SetInt(String::ToInteger(node->GetChildAt(1)->ToString()));
}

void ShaderParser::ReadVec3Property(SyntaxNode* node, Property* property) {
	glm::vec3 value;
	ReadVec3(value, node);
	property->value.SetVector3(value);
}

void ShaderParser::ReadVec4Property(SyntaxNode* node, Property* property) {
	glm::vec4 value;
	ReadVec4(value, node);
	property->value.SetVector4(value);
}

void ShaderParser::ReadColorProperty(SyntaxNode* node, Property* property) {
	Color value = Color::white;
	ReadVec4(*(glm::vec4*)&value, node);
	property->value.SetColor(value);
}

void ShaderParser::ReadTexture2DProperty(SyntaxNode* node, Property* property) {
	std::string value;
	ReadString(value, node);

	Texture2D texture;
	if (value.empty() || value == "white") {
		texture = Resources::instance()->GetWhiteTexture();
	}
	else if (value == "black") {
		texture = Resources::instance()->GetBlackTexture();
	}

	if (texture) {
		property->value.SetTexture(texture);
	}
	else {
		Debug::LogError("invalid texture2D property %s.", value.c_str());
	}
}

void ShaderParser::ReadMat3Property(SyntaxNode* node, Property* property) {
	property->value.SetMatrix3(glm::mat3(0));
}

void ShaderParser::ReadMat4Property(SyntaxNode* node, Property* property) {
	property->value.SetMatrix4(glm::mat4(0));
}

void ShaderParser::ReadProperty(SyntaxNode* node, Property* property) {
	const std::string& ns = node->ToString();
	property->name = node->GetChildAt(0)->ToString();

	if (ns == "Int") {
		ReadIntProperty(node, property);
	}
	else if (ns == "Single") {
		ReadFloatProperty(node, property);
	}
	else if (ns == "Vec3") {
		ReadVec3Property(node, property);
	}
	else if (ns == "Vec4") {
		ReadVec4Property(node, property);
	}
	else if (ns == "Color") {
		ReadColorProperty(node, property);
	}
	else if (ns == "Texture2D") {
		ReadTexture2DProperty(node, property);
	}
	else if (ns == "Mat3") {
		ReadMat3Property(node, property);
	}
	else if (ns == "Mat4") {
		ReadMat4Property(node, property);
	}
	else if (ns == "RangedInt") {
		ReadRangedInt(node, property);
	}
	else if (ns == "RangedFloat") {
		ReadRangedFloat(node, property);
	}
	else {
		Debug::LogError("invalid property type %s.", ns.c_str());
	}
}

void ShaderParser::ReadPropertyBlock(SyntaxNode* node, std::vector<Property*>& properties) {
	if (node == nullptr) { return; }

	SyntaxNode* c = node->GetChildAt(0);
	if (c != nullptr) {
		ReadProperties(c, properties);
	}
}

void ShaderParser::ReadTag(SyntaxNode* node, Semantics::Tag& tag) {
	tag.key = node->ToString();
	tag.value = node->GetChildAt(0)->ToString();
}

void ShaderParser::ReadTagBlock(SyntaxNode* node, std::vector<Semantics::Tag>& tags) {
	SyntaxNode* c = node->GetChildAt(0);
	if (c != nullptr) {
		ReadTags(c, tags);
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
