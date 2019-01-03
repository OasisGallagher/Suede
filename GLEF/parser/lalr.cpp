#include <sstream>
#include <algorithm>

#include "lr0.h"
#include "lalr.h"
#include "lrtable.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "glefdefines.h"
#include "os/filesystem.h"
#include "grammar/grammar.h"

struct Ambiguity {
	int state;
	GrammarSymbolPtr symbol;
	LRAction first;
	LRAction second;

	bool operator == (const Ambiguity& other) const;
};

class Ambiguities {
	typedef std::vector<Ambiguity> container_type;
public:
	void clear();
	bool insert(const Ambiguity& ambiguity);

private:
	container_type container_;
};

LALR::LALR() :coreItemsCount_(0){
	tmp_ = new LR1Item;
	ambiguities_ = new Ambiguities;
}

LALR::~LALR() {
	delete ambiguities_;
}

void LALR::Setup(Environment* env, FirstSetTable* firstSets) {
	env_ = env;
	firstSets_ = firstSets;
}

bool LALR::Parse(LRActionTable& actionTable, LRGotoTable& gotoTable) {
	//Debug::Log("create LR0 itemsets");
	LR0 lr0;
	lr0.Setup(env_, firstSets_);
	lr0.CreateLR0Itemsets(itemsets_, edges_);
	
	coreItemsCount_ = 0;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		coreItemsCount_ += std::count_if((*ite)->begin(), (*ite)->end(), [](const LR1ItemPtr& item) { return item->IsCore(); });
	}

	LR1ItemPtr init = *(*itemsets_.begin())->begin();
	init->GetForwards().insert(NativeSymbols::zero);

	//Debug::Log("calculate forwards and propagations");
	CalculateForwardsAndPropagations();

	//Debug::Log("propagate forwards");
	PropagateSymbols();

	//Debug::Log("create parsing table");
	bool status = CreateLRParsingTable(gotoTable, actionTable);

	return status;
}

bool LALR::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	ambiguities_->clear();
	if (!CreateGotoTable(gotoTable)) {
		return false;
	}

	return CreateActionTable(actionTable);
}

bool LALR::CreateActionTable(LRActionTable &actionTable) {
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		const LR1ItemsetPtr& itemset = *ite;
		for (LR1Itemset::const_iterator ite2 = itemset->begin(); ite2 != itemset->end(); ++ite2) {
			const LR1ItemPtr& item = *ite2;
			ParseLRAction(actionTable, itemset, item);
		}
	}

	return true;
}

bool LALR::InsertActionTable(LRActionTable& actionTable, const LR1ItemsetPtr& src, const GrammarSymbolPtr& symbol, const LRAction& action) {
	int state = String::ToInteger(src->GetName());
	LRActionTable::ib_pair status = actionTable.insert(state, symbol, action);
	if (!status.second && status.first->second != action) {
		Ambiguity ambiguity = { state, symbol, status.first->second, action };

		// ambiguity does not exist.
		if (ambiguities_->insert(ambiguity)) {
			std::string prompt = String::Format("CONFLICT at (%d, %s). Replace %s with %s ?",
				state, symbol->ToString().c_str(), 
				status.first->second.ToString(env_->grammars).c_str(), 
				action.ToString(env_->grammars).c_str());
			
			// Conflict.
			Debug::LogError(prompt.c_str());
			return false;
		}
	}

	return true;
}

bool LALR::ParseLRAction(LRActionTable & actionTable, const LR1ItemsetPtr& itemset, const LR1ItemPtr &item) {
	Grammar* g = nullptr;
	const Condinate* cond = env_->grammars.GetTargetCondinate(item->GetCpos(), &g);

	if (g->GetLhs() == NativeSymbols::program && item->GetDpos() == 1
		&& item->GetForwards().size() == 1 && *item->GetForwards().begin() == NativeSymbols::zero) {
		LRAction action = { LRActionAccept };
		return InsertActionTable(actionTable, itemset, NativeSymbols::zero, action);
	}

	if (item->GetDpos() >= (int)cond->symbols.size() || cond->symbols.front() == NativeSymbols::epsilon) {
		bool status = true;

		if (g->GetLhs() != NativeSymbols::program) {
			LRAction action = { LRActionReduce, item->GetCpos() };
			for (Forwards::const_iterator fi = item->GetForwards().begin(); fi != item->GetForwards().end(); ++fi) {
				status = InsertActionTable(actionTable, itemset, *fi, action) || status;
			}
		}

		return status;
	}

	const GrammarSymbolPtr& symbol = cond->symbols[item->GetDpos()];
	if (symbol->SymbolType() == GrammarSymbolTerminal) {
		LR1ItemsetPtr target = new LR1Itemset;
		if (edges_.get(itemset, symbol, target)) {
			int j = String::ToInteger(target->GetName());
			LRAction action = { LRActionShift, j };
			return InsertActionTable(actionTable, itemset, symbol, action);
		}
	}

	return true;
}


bool LALR::CreateGotoTable(LRGotoTable &gotoTable) {
	for (LR1EdgeTable::const_iterator ite = edges_.begin(); ite != edges_.end(); ++ite) {
		const GrammarSymbolPtr& symbol = ite->first.second;
		if (symbol->SymbolType() == GrammarSymbolNonterminal) {
			int i = String::ToInteger(ite->first.first->GetName());
			int j = String::ToInteger(ite->second->GetName());
			gotoTable.insert(i, symbol, j);
		}
	}

	return true;
}

void LALR::CalculateLR1Itemset(LR1ItemsetPtr& answer, LR1ItemsetPtr& itemset) {
	for (; CalculateLR1ItemsetOnePass(answer, itemset);) {
	}
}

bool LALR::CalculateLR1ItemsetOnePass(LR1ItemsetPtr& answer, LR1ItemsetPtr& itemset) {
	LR1ItemsetPtr newItems = new LR1Itemset;

	for (LR1Itemset::iterator isi = answer->begin(); isi != answer->end(); ++isi) {
		const LR1ItemPtr& current = *isi;
		const Condinate* cond = env_->grammars.GetTargetCondinate(current->GetCpos(), nullptr);

		if (current->GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbolPtr& lhs = cond->symbols[current->GetDpos()];

		if (lhs->SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		AddLR1Items(newItems, lhs, current, itemset);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems->begin(); ite != newItems->end(); ++ite) {
		setChanged = answer->insert(*ite) || setChanged;
	}

	return setChanged;
}

void LALR::AddLR1Items(LR1ItemsetPtr &answer, const GrammarSymbolPtr& lhs, const LR1ItemPtr &current, LR1ItemsetPtr& itemset) {
	int gi = 1;
	GrammarSymbolSet firstSet;
	
	Grammar* grammar = env_->grammars.FindGrammar(lhs, &gi);
	const Condinate* srcCond = env_->grammars.GetTargetCondinate(current->GetCpos(), nullptr);

	SymbolVector beta(srcCond->symbols.begin() + current->GetDpos() + 1, srcCond->symbols.end());
	beta.push_back(NativeSymbols::null);

	int condinateIndex = 0;
	const CondinateContainer& conds = grammar->GetCondinates();

	for (CondinateContainer::const_iterator ci = conds.begin(); ci != conds.end(); ++ci, ++condinateIndex) {
		const Condinate* tc = *ci;
		Forwards forwards = current->GetForwards();
		for (Forwards::const_iterator fi = forwards.begin(); fi != forwards.end(); ++fi) {
			beta.back() = *fi;
			firstSets_->GetFirstSet(firstSet, beta.begin(), beta.end());

			for (GrammarSymbolSet::const_iterator fsi = firstSet.begin(); fsi != firstSet.end(); ++fsi) {
				SymbolVector::const_iterator ite = tc->symbols.begin();
				int dpos = 0;

				for (; ite != tc->symbols.end(); ++ite, ++dpos) {
					LR1ItemPtr newItem = FindItem(Math::MakeDword(condinateIndex, gi), dpos, itemset);
					newItem->GetForwards().insert(*fsi);
					answer->insert(newItem);

					if (*ite == NativeSymbols::epsilon || !IsNullable(*ite)) {
						break;
					}
				}

				if (ite == tc->symbols.end()) {
					LR1ItemPtr newItem = FindItem(Math::MakeDword(condinateIndex, gi), dpos, itemset);
					newItem->GetForwards().insert(*fsi);
					answer->insert(newItem);
				}
			}

			firstSet.clear();
		}
	}
}

bool LALR::IsNullable(const GrammarSymbolPtr& symbol) {
	GrammarSymbolSet& firsts = firstSets_->at(symbol);
	return firsts.find(NativeSymbols::epsilon) != firsts.end();
}

LR1ItemPtr LALR::FindItem(int cpos, int dpos, LR1ItemsetPtr& dict) {
	tmp_->SetCpos(cpos);
	tmp_->SetDpos(dpos);
	LR1Itemset::iterator pos = dict->find(tmp_);
	if (pos == dict->end()) {
		Debug::LogError("can not find item.");
		return new LR1Item;
	}

	return *pos;
}

std::string LALR::ToString() const {
	std::ostringstream oss;
	
	oss << String::Heading(" LR1 Edges ") << "\n";
	oss << edges_.ToString(env_->grammars);

	oss << "\n\n";

	oss << String::Heading(" LR1 Itemsets ") << "\n";
	oss << itemsets_.ToString(env_->grammars);

	return oss.str();
}

void LALR::PropagateSymbols() {
	for (; PropagateSymbolsOnePass();) {
	}
}

bool LALR::PropagateSymbolsOnePass() {
	bool propagated = false;

	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (LR1Itemset::iterator ii = (*ite)->begin(); ii != (*ite)->end(); ++ii) {
			propagated = PropagateFrom(*ii) || propagated;
		}
	}

	return propagated;
}

bool LALR::PropagateFrom(const LR1ItemPtr &src) {
	Propagations::iterator pos = propagations_.find(src);
	if (pos == propagations_.end()) {
		return false;
	}

	bool propagated = false;
	const Forwards& forwards = src->GetForwards();
	
	LR1ItemsetPtr& itemset = (LR1ItemsetPtr&)pos->second;
	for (LR1Itemset::iterator is = itemset->begin(); is != itemset->end(); ++is) {
		LR1ItemPtr& target = (LR1ItemPtr&)*is;
		for (Forwards::const_iterator fi = forwards.begin(); fi != forwards.end(); ++fi) {
			propagated = target->GetForwards().insert(*fi) || propagated;
		}
	}

	return propagated;
}

void LALR::CalculateForwardsAndPropagations() {
	//Debug::Log("add forwards and propagations");
	int index = 1;
	LR1ItemsetPtr target = nullptr;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		LR1ItemsetPtr& dict = (LR1ItemsetPtr&)*ite;
		
		for (LR1Itemset::iterator ii = dict->begin(); ii != dict->end(); ++ii) {
			LR1ItemPtr item = *ii;
			if (!item->IsCore()) {
				continue;
			}

			LR1ItemsetPtr itemset = new LR1Itemset;

			item->GetForwards().insert(NativeSymbols::unknown);
			itemset->insert(item);

			CalculateLR1Itemset(itemset, dict);

			GrammarSymbolContainer::iterator si = env_->terminalSymbols.begin();
			for (; si != env_->terminalSymbols.end(); ++si) {
				if (edges_.get(dict, si->second, target)) {
					AddForwardsAndPropagations(item, itemset, target, si->second);
				}
			}

			si = env_->nonterminalSymbols.begin();
			for (; si != env_->nonterminalSymbols.end(); ++si) {
				if (si->second != NativeSymbols::program && edges_.get(dict, si->second, target)) {
					AddForwardsAndPropagations(item, itemset, target, si->second);
				}
			}

			++index;
		}
	}

	//Debug::Log("clean up");
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (LR1Itemset::iterator ii = (*ite)->begin(); ii != (*ite)->end(); ++ii) {
			Forwards& forwards = (Forwards&)(*ii)->GetForwards();
			forwards.erase(NativeSymbols::unknown);
		}
	}
}

void LALR::AddForwardsAndPropagations(LR1ItemPtr& src, const LR1ItemsetPtr& itemset, LR1ItemsetPtr& dict, const GrammarSymbolPtr& symbol) {
	for (LR1Itemset::iterator ite = itemset->begin(); ite != itemset->end(); ++ite) {
		const LR1ItemPtr& item = *ite;
		const Condinate* cond = env_->grammars.GetTargetCondinate(item->GetCpos(), nullptr);
		if (item->GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		if (cond->symbols.front() == NativeSymbols::epsilon) {
			continue;
		}

		if (cond->symbols[item->GetDpos()] != symbol) {
			continue;
		}

		LR1ItemPtr target = FindItem(item->GetCpos(), item->GetDpos() + 1, dict);
		const Forwards& forwards = item->GetForwards();
		for (Forwards::const_iterator ite2 = forwards.begin(); ite2 != forwards.end(); ++ite2) {
			if (*ite2 == NativeSymbols::unknown) {
				LR1ItemsetPtr& itemset = propagations_[src];
				if (!itemset) {
					itemset = new LR1Itemset;
				}

				itemset->insert(target);
			}
			else {
				target->GetForwards().insert(*ite2);
			}
		}
	}
}

bool Ambiguity::operator==(const Ambiguity& other) const {
	if (state != other.state || symbol != other.symbol) {
		return false;
	}

	return (first == other.first && second == other.second) 
		|| (first == other.second && second == other.first);
}

bool Ambiguities::insert(const Ambiguity& ambiguity) {
	if (std::find(container_.begin(), container_.end(), ambiguity) == container_.end()) {
		container_.push_back(ambiguity);
		return true;
	}

	return false;
}

void Ambiguities::clear() {
	container_.clear();
}
