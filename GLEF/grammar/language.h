#pragma once
#include <string>

class Syntaxer;
class SyntaxTree;
class SourceScanner;

struct Environment;

class Language {
public:
	Language();
	~Language();

public:
	bool SaveSyntaxer(const char* savePath);
	bool LoadSyntaxer(const char* savePath);
	bool BuildSyntaxer(const char* grammars);

public:
	bool Parse(SyntaxTree* tree, const std::string& path);
	std::string ToString() const;

private:
	bool SetupEnvironment(const char* grammars);
	bool ParseProductions(const char* grammars);

private:
	Environment* env_;
	Syntaxer* syntaxer_;
	SourceScanner* scanner_;
};
