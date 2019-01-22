#pragma once

#include "lr1.h"
#include "lrimpl.h"
#include "grammar/grammar.h"

struct Environment;

class LR0 {
public:
	LR0();
	~LR0();

public:
	std::string ToString() const;
	void Setup(Environment* env, FirstSetTable* firstSets);
	bool CreateLR0Itemsets(LR1ItemsetContainer& itemsets, LR1EdgeTable& edges);

private:
	void CalculateClosure(LR1ItemsetPtr& answer);
	bool IsNullable(const GrammarSymbolPtr& symbol);
	void AddLR1Items(LR1ItemsetPtr& answer, const GrammarSymbolPtr& lhs);
	bool CalculateClosureOnePass(LR1ItemsetPtr& answer);
	bool CalculateLR1EdgeTarget(LR1ItemsetPtr& answer, const LR1ItemsetPtr& src, const GrammarSymbolPtr& symbol);
	bool GetLR1EdgeTarget(LR1ItemsetPtr& answer, const LR1ItemsetPtr& src, const GrammarSymbolPtr& symbol);
	bool CreateLR1ItemsetsOnePass();

private:
	Environment* env_;
	FirstSetTable* firstSets_;

	LR1EdgeTable edges_;
	LR1ItemsetContainer itemsets_;
};
