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
	void Setup(const char* productions, const char* fileName);

public:
	bool Parse(SyntaxTree* tree, const std::string& file);
	std::string ToString() const;

private:
	void SaveSyntaxer(const char* fileName);
	void LoadSyntaxer(const char* fileName);
	void BuildSyntaxer(const char* source);

	bool SetupEnvironment(const char* source);
	bool ParseProductions(const char* source);

private:
	Environment* env_;
	Syntaxer* syntaxer_;
};
