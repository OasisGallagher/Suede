#pragma once
#include <vector>
#include "grammarsymbol.h"
#include "tools/noncopyable.h"

class Action;

struct Condinate : NonCopyable {
	Condinate(const SymbolVector& container, const std::string& actionText);
	~Condinate();

	std::string ToString() const;

	SymbolVector symbols;
	Action* action;
};

class CondinateContainer : public std::vector<Condinate*> {
public:
	std::string ToString() const;
};

class Grammar {
public:
	Grammar();
	Grammar(const GrammarSymbol& lhs);
	~Grammar();

public:
	void SetLhs(const GrammarSymbol& symbol);
	void AddCondinate(const std::string& action, const SymbolVector& symbols);

	const GrammarSymbol& GetLhs() const;
	const CondinateContainer& GetCondinates() const;

	std::string ToString() const;

private:
	GrammarSymbol lhs_;
	CondinateContainer condinates_;
};

class GrammarContainer : public std::vector<Grammar*> {
public:
	Grammar* FindGrammar(const GrammarSymbol& lhs, int* index);
	const Condinate* GetTargetCondinate(int cpos, Grammar** grammar) const;
};
