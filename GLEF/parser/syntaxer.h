#pragma once

#include "lrtable.h"
#include "containers/table.h"

#include "grammar/grammar.h"
#include "grammar/grammarsymbol.h"

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
class BooleanTable;

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
public:
	void SetLineNumber(uint value) { lineno_ = value; }
	uint GetLineNumber() const { return lineno_; }

private:
	uint lineno_;
};

class Integer : public SyntaxElement {
public:
	virtual void SetText(const std::string& value);
};

class Single : public SyntaxElement {
public:
	virtual void SetText(const std::string& value);
};

class Boolean : public SyntaxElement {
public:
	virtual void SetText(const std::string& value);
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
	void Shift(int state, void* addr, const GrammarSymbolPtr& symbol);
	bool Error(const GrammarSymbolPtr& symbol, const TokenPosition& position);

	bool CreateSyntaxTree(SyntaxNode*& root, SourceScanner* sourceScanner);

	void CleanupOnFailure();

	GrammarSymbolPtr FindSymbol(const ScannerToken& token, const TokenPosition& position, void*& addr);
	GrammarSymbolPtr ParseNextSymbol(TokenPosition& position, void*& addr, SourceScanner* sourceScanner);

private:
	SyntaxerStack* stack_;
	SyntaxerSetupParameter p_;

private:
	SymTable* symTable_;
	CodeTable* codeTable_;
	LiteralTable* literalTable_;
	IntegerTable* integerTable_;
	SingleTable* singleTable_;
	BooleanTable* booleanTable_;
};
