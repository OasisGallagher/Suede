#pragma once
#include "lrimpl.h"
#include "containers/table.h"

class LRGotoTable : public Table <int, GrammarSymbol, int> {
public:
	std::string ToString() const;
};

class LRActionTable : public Table <int, GrammarSymbol, LRAction> {
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
	int GetGoto(int current, const GrammarSymbol& symbol);
	LRAction GetAction(int current, const GrammarSymbol& symbol);
	
	std::string ToString(const GrammarContainer& grammars) const;

private:
	LRGotoTable gotoTable_;
	LRActionTable actionTable_;
};
