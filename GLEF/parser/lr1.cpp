#include <sstream>
#include <algorithm>

#include "lr1.h"
#include "math/mathf.h"
#include "tools/string.h"
#include "glefdefines.h"
#include "grammar/grammar.h"

bool Forwards::operator <(const Forwards& other) const {
	const_iterator first1 = begin(), first2 = other.begin();
	for (; first1 != end() && first2 != other.end(); ++first1, ++first2) {
		if (*first1 != *first2) {
			return *first1 < *first2;
		}
	}

	if (first1 == end() && first2 == other.end()) {
		return false;
	}

	return first1 == end();
}

void Forwards::erase(const GrammarSymbolPtr& symbol) {
	iterator pos = cont_.find(symbol);
	if (pos != cont_.end()) {
		cont_.erase(pos);
	}
}

bool Forwards::insert(const GrammarSymbolPtr& symbol) {
	return cont_.insert(symbol).second;
}

int LR1Item::Compare(const LR1Item& other) const {
	int c = cpos_ - other.cpos_;
	if (c != 0) {
		return Mathf::Sign(c);
	}

	return Mathf::Sign(dpos_ - other.dpos_);
}

std::string LR1Item::ToRawString() const {
	return String::Format("(%d, %d, %d)", Mathf::Highword(cpos_), Mathf::Loword(cpos_), dpos_);
}

std::string LR1Item::ToString(const GrammarContainer& grammars) const {
	Grammar* g = nullptr;
	const Condinate* cond = grammars.GetTargetCondinate(cpos_, &g);

	std::ostringstream oss;
	oss << g->GetLhs()->ToString() << " : ";

	const char* seperator = "";
	for (size_t i = 0; i < cond->symbols.size(); ++i) {
		if (i == dpos_) {
			oss << seperator;
			seperator = " ";
			oss << "¡¤";
		}

		oss << seperator;
		oss << cond->symbols[i]->ToString();
		seperator = " ";
	}

	if (dpos_ == (int)cond->symbols.size()) {
		oss << seperator << "¡¤";
	}

	oss << ", ";
	oss << "( " << String::Concat(forwards_.begin(), forwards_.end(), [](const GrammarSymbolPtr& x) { return x->ToString(); }, "/") << " )";

	return oss.str();
}

int LR1Itemset::Compare(const LR1Itemset& other) const {
	const_iterator first1 = begin(), first2 = other.begin();
	for (; first1 != end() && first2 != other.end(); ++first1, ++first2) {
		int c = (*first1)->Compare(**first2);
		if (c != 0) { return c; }
	}

	if (first1 == end() && first2 == other.end()) {
		return 0;
	}

	return first1 == end() ? -1 : 1;
}

bool LR1Itemset::insert(const LR1ItemPtr& item) {
	std::pair<iterator, bool> state = container_.insert(item);
	if (state.second) {
		return true;
	}

	bool result = false;

	LR1ItemPtr& old = (LR1ItemPtr&)*state.first;
	const Forwards& forwards = item->GetForwards();
	for (Forwards::const_iterator ite = forwards.begin(); ite != forwards.end(); ++ite) {
		result = old->GetForwards().insert(*ite) || result;
	}

	return result;
}

const std::string& LR1Itemset::GetName() const {
	return name_;
}

void LR1Itemset::SetName(const std::string& name) {
	name_ = name;
}

std::string LR1Itemset::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	oss << "(" << GetName() << ") ";
	oss << "{ ";

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		if (!(*ite)->IsCore()) {
			continue;
		}

		oss << seperator;
		seperator = ", ";
		oss << "{ " << (*ite)->ToString(grammars) << " }";
	}

	oss << " }";

	return oss.str();
}

std::string LR1ItemsetContainer::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	const char* seperator = "";
	int index = 0;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << (*ite)->ToString(grammars);
	}

	return oss.str();
}

std::string Propagations::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << ite->first->ToString(grammars) << " >> ( ";
		const char* seperator2 = "";
		for (LR1Itemset::const_iterator ite2 = ite->second->begin();
			ite2 != ite->second->end(); ++ite2) {
			oss << seperator2;
			seperator2 = ", ";
			oss << (*ite2)->ToString(grammars);
		}

		oss << " )";
	}

	return oss.str();
}

std::string LR1EdgeTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		oss << "( ";
		oss << ite->first.first->GetName();
		oss << ", ";
		oss << (*ite->first.second).ToString();
		oss << " )";

		oss << " => ";
		oss << ite->second->GetName();
	}

	return oss.str();
}
