#pragma once
#include "lr1.h"
#include "lrimpl.h"
#include "parser.h"

struct Condinate;

class LR0;
class Grammar;
class Ambiguities;

class LALR : public LRImpl {
public:
	LALR();
	~LALR();

public:
	virtual std::string ToString() const;

	virtual bool Parse(LRActionTable& actionTable, LRGotoTable& gotoTable);

	virtual void Setup(Environment* env, FirstSetTable* firstSet);

protected:
	virtual bool CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable);

	bool CreateGotoTable(LRGotoTable &gotoTable);
	bool CreateActionTable(LRActionTable &actionTable);
	bool InsertActionTable(LRActionTable &actionTable, const LR1ItemsetPtr& src, const GrammarSymbolPtr& symbol, const LRAction& action);

private:
	bool IsNullable(const GrammarSymbolPtr& symbol);

	void PropagateSymbols();
	bool PropagateSymbolsOnePass();
	bool PropagateFrom(const LR1ItemPtr& src);

	LR1ItemPtr FindItem(int cpos, int dpos, LR1ItemsetPtr& dict);

	void CalculateForwardsAndPropagations();
	void AddForwardsAndPropagations(LR1ItemPtr& item, const LR1ItemsetPtr& itemset, LR1ItemsetPtr& dict, const GrammarSymbolPtr& symbol);

	void CalculateLR1Itemset(LR1ItemsetPtr& answer, LR1ItemsetPtr& itemset);
	bool CalculateLR1ItemsetOnePass(LR1ItemsetPtr& answer, LR1ItemsetPtr& itemset);

	void AddLR1Items(LR1ItemsetPtr &answer, const GrammarSymbolPtr& lhs, const LR1ItemPtr &current, LR1ItemsetPtr& itemset);

	bool ParseLRAction(LRActionTable & actionTable, const LR1ItemsetPtr& itemset, const LR1ItemPtr &item);

private:
	Environment* env_;
	Ambiguities* ambiguities_;
	FirstSetTable* firstSets_;

	LR1ItemPtr tmp_;
	LR1EdgeTable edges_;

	int coreItemsCount_;
	Propagations propagations_;
	LR1ItemsetContainer itemsets_;
};
