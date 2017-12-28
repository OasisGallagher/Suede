#pragma once
#include <string>
#include "compiler_defines.h"

class Syntaxer;
class SyntaxTree;
class TextScanner;

struct Environment;

class COMPILER_API Language {
public:
	Language();
	~Language();

public:
	void Setup(const char* grammars, const char* savePath);

public:
	bool Parse(SyntaxTree* tree, const std::string& path);
	std::string ToString() const;

private:
	void SaveSyntaxer(const char* savePath);
	void LoadSyntaxer(const char* savePath);
	void BuildSyntaxer(const char* grammars);

	bool SetupEnvironment(const char* grammars);
	bool ParseProductions(const char* grammars);

private:
	Environment* env_;
	Syntaxer* syntaxer_;
};
