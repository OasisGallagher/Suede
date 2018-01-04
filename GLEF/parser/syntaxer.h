#pragma once

#include "grammar.h"
#include "lrtable.h"
#include "grammarsymbol.h"
#include "containers/table.h"

class SyntaxNode;
class SyntaxTree;
class SourceScanner;

struct Environment;
struct TokenPosition;
struct SyntaxerStack;

class SymTable;
class CodeTable;
class LiteralTable;
class IntegerTable;
class SingleTable;

struct SyntaxerSetupParameter {
	Environment* env;
	LRTable lrTable;
};

class SyntaxElement {
public:
	virtual void SetText(const std::string& value) { value_ = value; }

public:
	std::string ToString() const { return value_; }

protected:
	std::string value_;
};

class Sym : public SyntaxElement {
};

class Literal : public SyntaxElement {
};

class Code : public SyntaxElement {
};

class Integer : public SyntaxElement {
public:
	void SetText(const std::string& value);
};

class Single : public SyntaxElement {
public:
	void SetText(const std::string& value);
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
	bool ParseSyntax(SyntaxTree* tree, SourceScanner* sourceScanner);

public:
	std::string ToString() const;

private:
	void Clear();

	int Reduce(int cpos);
	void Shift(int state, void* addr, const GrammarSymbol& symbol);
	bool Error(const GrammarSymbol& symbol, const TokenPosition& position);

	bool CreateSyntaxTree(SyntaxNode*& root, SourceScanner* sourceScanner);

	void CleanupOnFailure();

	GrammarSymbol FindSymbol(const ScannerToken& token, void*& addr);
	GrammarSymbol ParseNextSymbol(TokenPosition& position, void*& addr, SourceScanner* sourceScanner);

private:
	SyntaxerStack* stack_;
	SyntaxerSetupParameter p_;

private:
	SymTable* symTable_;
	CodeTable* codeTable_;
	LiteralTable* literalTable_;
	IntegerTable* integerTable_;
	SingleTable* singleTable_;
};
