#pragma once

#include "grammar.h"
#include "lrtable.h"
#include "grammarsymbol.h"
#include "containers/table.h"

class SyntaxNode;
class SyntaxTree;
class FileScanner;

struct Environment;
struct TokenPosition;
struct SyntaxerStack;

class SymTable;
class CodeTable;
class LiteralTable;
class IntegerTable;

struct SyntaxerSetupParameter {
	Environment* env;
	LRTable lrTable;
};

class Sym {
public:
	void SetText(const std::string& value) { value_ = value; }
	std::string ToString() const;

private:
	std::string value_;
};

class Literal {
public:
	void SetText(const std::string& value) { value_ = value; }
	std::string ToString() const;

private:
	std::string value_;
};

class Code {
public:
	void SetText(const std::string& value) { value_ = value; }
	std::string ToString() const;

private:
	std::string value_;
};

class Integer {
public:
	void SetText(const std::string& value);
	std::string ToString() const;

private:
	int value_;
};

class Syntaxer {
public:
	Syntaxer();
	~Syntaxer();

public:
	bool Load(std::ifstream& file);
	bool Save(std::ofstream& file);

public:
	void Setup(const SyntaxerSetupParameter& p);
	bool ParseSyntax(SyntaxTree* tree, FileScanner* fileScanner);

public:
	std::string ToString() const;

private:
	void Clear();

	int Reduce(int cpos);
	void Shift(int state, void* addr, const GrammarSymbol& symbol);
	bool Error(const GrammarSymbol& symbol, const TokenPosition& position);

	bool CreateSyntaxTree(SyntaxNode*& root, FileScanner* fileScanner);

	void CleanupOnFailure();

	GrammarSymbol FindSymbol(const ScannerToken& token, void*& addr);
	GrammarSymbol ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner);

private:
	SyntaxerStack* stack_;
	SyntaxerSetupParameter p_;

private:
	SymTable* symTable_;
	CodeTable* codeTable_;
	LiteralTable* literalTable_;
	IntegerTable* integerTable_;
};
