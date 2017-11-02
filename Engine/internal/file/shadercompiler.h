#pragma once

#include "internal/base/shaderinternal.h"

class ShaderCompiler {
public:
	bool Compile(const std::string& path, const std::string& defines, std::string(&answer)[ShaderTypeCount]);

private:
	void Clear();
	bool ReadShaderSource(const std::vector<std::string> &lines);
	std::string FormatDefines(const std::string& defines);
	bool CompileShaderSource(const std::vector<std::string>& lines, const std::string& defines);

	bool Preprocess(const std::string& line);
	bool PreprocessInclude(const std::string& parameter);
	bool PreprocessShader(const std::string& parameter);

	void CalculateDefinesPermutations(std::vector<std::string>& anwser);

	ShaderType ParseShaderType(const std::string& line);

private:
	ShaderType type_;
	std::string path_;
	std::string* answer_;
	std::string globals_;
	std::string source_;
};
