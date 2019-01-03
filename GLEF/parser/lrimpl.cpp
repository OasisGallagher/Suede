#include <sstream>

#include "lrimpl.h"
#include "lrtable.h"
#include "debug/debug.h"

#include "grammar/grammar.h"
#include "grammar/grammarsymbol.h"

static const char* actionTexts[] = { "err", "s", "r", "acc" };

bool LRAction::operator == (const LRAction& other) const {
	return type == other.type && parameter == other.parameter;
}

bool LRAction::operator != (const LRAction& other) const {
	return type != other.type || parameter != other.parameter;
}

std::string LRAction::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	oss << "(";
	oss << actionTexts[type];

	if (type == LRActionShift) {
		oss << parameter;
	}
	else if (type == LRActionReduce) {
		oss << parameter;
		/*
		oss << "(";
		Grammar* g = nullptr;
		const Condinate* cond = grammars.GetTargetCondinate(actionParameter, &g);
		oss << g->GetLhs().ToString() + " : " + cond->ToString();
		oss << ")";
		*/
	}

	oss << ")";

	return oss.str();
}

std::string LRGotoTable::ToString() const {
	const char* seperator = "";
	std::ostringstream oss;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << "(";
		oss << ite->first.first;
		oss << ", ";
		oss << ite->first.second->ToString();
		oss << ")";
		oss << " => ";
		oss << ite->second;
	}

	return oss.str();
}

std::string LRActionTable::ToString(const GrammarContainer& grammars) const {
	const char* seperator = "";
	std::ostringstream oss;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << "(";
		oss << ite->first.first;
		oss << ", ";
		oss << ite->first.second->ToString();
		oss << ")";
		oss << " => ";
		oss << ite->second.ToString(grammars);
	}

	return oss.str();
}
