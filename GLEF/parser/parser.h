#pragma once
#include "grammar/grammar.h"

class Syntaxer;
class SyntaxTree;
class SourceScanner;
class TextScanner;

struct ScannerToken;

struct Environment {
	GrammarContainer grammars;
	GrammarSymbolContainer terminalSymbols;
	GrammarSymbolContainer nonterminalSymbols;

	bool Load(std::ifstream& file);
	bool Save(std::ofstream& file);

	~Environment();
};

class Parser {
public:
	Parser();
	virtual ~Parser();

public:
	virtual std::string ToString() const;

public:
	bool Setup(Syntaxer& syntaxer, Environment* env);

protected:
	virtual bool ParseGrammars(Syntaxer& syntaxer, Environment* env) = 0;
	virtual void Clear();

protected:
	Grammar* FindGrammar(const GrammarSymbolPtr& lhs, int* index = nullptr);
	GrammarSymbolPtr CreateSymbol(const std::string& text);
	bool MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src);

	void CreateFirstSets();
	void CreateFollowSets();

protected:
	Environment* env_;

	// Vn��first/follow����.
	FirstSetTable firstSetContainer_;
	GrammarSymbolSetTable followSetContainer_;

private:
	bool CreateFirstSetsOnePass();
	bool CreateFollowSetsOnePass();
};
