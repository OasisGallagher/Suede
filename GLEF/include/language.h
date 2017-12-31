#pragma once
#include <string>

class Syntaxer;
class SyntaxTree;
class FileScanner;

struct Environment;

class Language {
public:
	Language();
	~Language();

public:
	void SaveSyntaxer(const char* savePath);
	void LoadSyntaxer(const char* savePath);
	void BuildSyntaxer(const char* grammars);

public:
	bool Parse(SyntaxTree* tree, const std::string& path);
	std::string ToString() const;

private:
	
	bool SetupEnvironment(const char* grammars);
	bool ParseProductions(const char* grammars);

private:
	Environment* env_;
	Syntaxer* syntaxer_;
	FileScanner* scanner_;
};
