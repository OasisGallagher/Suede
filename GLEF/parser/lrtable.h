#pragma once
#include "lrimpl.h"
#include "containers/table.h"
#include "grammar/grammarsymbol.h"

class LRGotoTable : public table <int, GrammarSymbolPtr, int> {
public:
	std::string ToString() const;
};

class LRActionTable : public table <int, GrammarSymbolPtr, LRAction> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LRTable {
public:
	LRTable();
	~LRTable();

public:
	friend class Serializer;
	friend class LRParser;

public:
	int GetGoto(int current, const GrammarSymbolPtr& symbol);
	LRAction GetAction(int current, const GrammarSymbolPtr& symbol);
	
	std::string ToString(const GrammarContainer& grammars) const;

private:
	LRGotoTable gotoTable_;
	LRActionTable actionTable_;
};
