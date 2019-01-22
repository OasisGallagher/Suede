#include <sstream>

#include "lalr.h"
#include "lrtable.h"
#include "tools/string.h"

LRTable::LRTable() {
}

LRTable::~LRTable() {
}

LRAction LRTable::GetAction(int current, const GrammarSymbolPtr& symbol) {
	LRAction action = { LRActionError };
	actionTable_.get(current, symbol, action);
	return action;
}

int LRTable::GetGoto(int current, const GrammarSymbolPtr& symbol) {
	int answer = -1;
	gotoTable_.get(current, symbol, answer);
	return answer;
}

std::string LRTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	
	oss << String::Heading(" Action table ") << "\n";
	oss << actionTable_.ToString(grammars);

	oss << "\n\n";

	oss << String::Heading(" Goto table ") << "\n";
	oss << gotoTable_.ToString();

	return oss.str();
}
