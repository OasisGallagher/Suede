#include "variables.h"
#include "textloader.h"
#include "shadercompiler.h"
#include "tools/path.h"
#include "tools/debug.h"
#include "tools/string.h"
#include "internal/base/glsldefines.h"

bool ShaderCompiler::Compile(const std::string& path, const std::string& defines, std::string(&answer)[ShaderTypeCount]) {
	std::vector<std::string> lines;
	if (!TextLoader::Load(Path::GetResourceRootDirectory() + path, lines)) {
		return false;
	}

	Clear();

	answer_ = answer;
	path_ = path;
	return CompileShaderSource(lines, defines);
}

void ShaderCompiler::Clear() {
	type_ = ShaderTypeCount;
	source_.clear();
	globals_.clear();
	answer_ = nullptr;
}

bool ShaderCompiler::CompileShaderSource(const std::vector<std::string>& lines, const std::string& defines) {
	globals_ = "#version " GLSL_VERSION "\n";
	// TODO: constants.
	globals_ += "#define C_MAX_BONE_COUNT " + std::to_string(C_MAX_BONE_COUNT) + "\n";
	globals_ += FormatDefines(defines);
	ReadShaderSource(lines);

	AssertX(type_ != ShaderTypeCount, "invalid shader file");
	answer_[type_] = globals_ + source_;

	return true;
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

ShaderType ShaderCompiler::ParseShaderType(const std::string& tag) {
	for (size_t i = 0; i < ShaderTypeCount; ++i) {
		if (tag == ShaderInternal::Description((ShaderType)i).tag) {
			return (ShaderType)i;
		}
	}

	AssertX(false, std::string("unkown shader tag ") + tag);
	return ShaderTypeCount;
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
	ShaderType newType = ParseShaderType(parameter);

	if (newType != type_) {
		if (type_ == ShaderTypeCount) {
			globals_ += source_;
		}
		else {
			if (!answer_[type_].empty()) {
				Debug::LogError(std::string(ShaderInternal::Description(type_).name) + " already exists.");
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
