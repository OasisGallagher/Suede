#include "serializer.h"
#include "debug/debug.h"

#include "parser/parser.h"
#include "parser/lrtable.h"
#include "parser/syntaxer.h"

#include "grammar/action.h"
#include "grammar/grammar.h"
#include "grammar/grammarsymbol.h"

#include "os/filesystem.h"

bool Serializer::SaveEnvironment(std::ofstream& file, Environment* env) {
	if (!SaveSymbols(file, env->terminalSymbols)) {
		Debug::LogError("failed to save terminal symbols.");
		return false;
	}

	if (!SaveSymbols(file, env->nonterminalSymbols)) {
		Debug::LogError("failed to save non-terminal symbols.");
		return false;
	}

	if (!SaveGrammars(file, env->grammars)) {
		Debug::LogError("failed to save grammars.");
		return false;
	}

	return true;
}

bool Serializer::LoadEnvironment(std::ifstream& file, Environment* env) {
	if (!LoadSymbols(file, env->terminalSymbols)) {
		Debug::LogError("failed to load terminal symbols.");
		return false;
	}

	if (!LoadSymbols(file, env->nonterminalSymbols)) {
		Debug::LogError("failed to load non-terminal symbols.");
		return false;
	}

	NativeSymbols::Copy(env->terminalSymbols, env->nonterminalSymbols);

	if (!LoadGrammars(file, env->terminalSymbols, env->nonterminalSymbols, env->grammars)) {
		Debug::LogError("failed to load grammars.");
		return false;
	}

	return true;
}

bool Serializer::SaveSyntaxer(std::ofstream& file, const SyntaxerSetupParameter& p) {
	if (!SaveLRTable(file, p.lrTable)) {
		Debug::LogError("failed to save lr-table.");
		return false;
	}

	return true;
}

bool Serializer::LoadSyntaxer(std::ifstream& file, SyntaxerSetupParameter& p) {
	if (!LoadLRTable(file, p.env->terminalSymbols, p.env->nonterminalSymbols, p.lrTable)) {
		Debug::LogError("failed to save lr-table.");
		return false;
	}

	return true;
}

bool Serializer::SaveSymbols(std::ofstream& file, const GrammarSymbolContainer& cont) {
	typedef std::ios::pos_type pos_type;
	pos_type oldpos = file.tellp(), newpos;
	int count = 0;
	file.seekp(oldpos + (pos_type)sizeof(count));

	for (GrammarSymbolContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		if (NativeSymbols::IsNative(ite->second)) {
			continue;
		}

		++count;
		if (!FileSystem::WriteString(file, ite->first)) {
			return false;
		}
	}

	newpos = file.tellp();
	file.seekp(oldpos);
	FileSystem::WriteInteger(file, count);

	file.seekp(newpos);

	return true;
}

bool Serializer::LoadSymbols(std::ifstream& file, GrammarSymbolContainer& cont) {
	int count = 0;
	if (!FileSystem::ReadInteger(file, &count)) {
		return false;
	}

	std::string text;
	for (int i = 0; i < count; ++i) {
		if (!FileSystem::ReadString(file, &text)) {
			return false;
		}

		GrammarSymbolPtr symbol = SymbolFactory::Create(text);
		cont.insert(std::make_pair(symbol->ToString(), symbol));
	}

	return true;
}

bool Serializer::SaveGrammars(std::ofstream& file, const GrammarContainer& cont) {
	FileSystem::WriteInteger(file, cont.size());

	for (GrammarContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		const Grammar* g = *ite;
		if (!FileSystem::WriteString(file, g->GetLhs()->ToString())) {
			return false;
		}

		const CondinateContainer& conds = g->GetCondinates();
		if (!FileSystem::WriteInteger(file, conds.size())) {
			return false;
		}

		for (CondinateContainer::const_iterator ite = conds.begin(); ite != conds.end(); ++ite) {
			const Condinate* c = *ite;
			if (!FileSystem::WriteInteger(file, c->symbols.size())) {
				return false;
			}

			for (SymbolVector::const_iterator ite2 = c->symbols.begin(); ite2 != c->symbols.end(); ++ite2) {
				if (!FileSystem::WriteString(file, (*ite2)->ToString())) {
					return false;
				}
			}

			if (!FileSystem::WriteString(file, c->action != nullptr ? c->action->ToString() : "")) {
				return false;
			}
		}
	}

	return true;
}

bool Serializer::LoadGrammars(std::ifstream& file, GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols, GrammarContainer& grammars) {
	int count = 0;
	if (!FileSystem::ReadInteger(file, &count)) {
		return false;
	}

	std::string ltext;
	for (int i = 0; i < count; ++i) {
		if (!FileSystem::ReadString(file, &ltext)) {
			return false;
		}

		GrammarSymbolPtr lhs = nonterminalSymbols[ltext];
		if (lhs == NativeSymbols::null) {
			Debug::LogError("can not find non-terminal symbol %s.", ltext.c_str());
			return false;
		}

		Grammar* grammar = new Grammar(lhs);
		grammars.push_back(grammar);

		if (!LoadCondinates(terminalSymbols, nonterminalSymbols, file, grammar)) {
			return false;
		}
	}

	return true;
}

bool Serializer::SaveLRTable(std::ofstream& file, const LRTable& table) {
	if (!SaveLRActionTable(file, table.actionTable_)) {
		return false;
	}

	return SaveLRGotoTable(file, table.gotoTable_);
}

bool Serializer::LoadLRTable(std::ifstream& file, GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols, LRTable& table) {
	if (!LoadLRActionTable(file, terminalSymbols, table)) {
		return false;
	}

	return LoadLRGotoTable(file, nonterminalSymbols, table);
}

bool Serializer::LoadCondinates(GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols, std::ifstream& file, Grammar* grammar) {
	int cn = 0;
	SymbolVector symbols;
	std::string stext, atext;

	if (!FileSystem::ReadInteger(file, &cn)) {
		return false;
	}

	for (int i = 0; i < cn; ++i) {
		int sn = 0;
		if (!FileSystem::ReadInteger(file, &sn)) {
			return false;
		}

		for (int j = 0; j < sn; ++j) {
			if (!FileSystem::ReadString(file, &stext)) {
				return false;
			}

			GrammarSymbolContainer::iterator pos = terminalSymbols.find(stext);
			if (pos == terminalSymbols.end() && (pos = nonterminalSymbols.find(stext)) == nonterminalSymbols.end()) {
				Debug::LogError("can not find symbol %s.", stext.c_str());
				return false;
			}

			GrammarSymbolPtr symbol = pos->second;
			symbols.push_back(symbol);
		}

		if (!FileSystem::ReadString(file, &atext)) {
			return false;
		}

		grammar->AddCondinate(atext, symbols);
		symbols.clear();
	}

	return true;
}

bool Serializer::SaveLRActionTable(std::ofstream& file, const LRActionTable &actionTable) {
	FileSystem::WriteInteger(file, actionTable.size());

	for (LRActionTable::const_iterator ite = actionTable.begin(); ite != actionTable.end(); ++ite) {
		if (!FileSystem::WriteInteger(file, ite->first.first)) {
			return false;
		}

		if (!FileSystem::WriteString(file, ite->first.second->ToString())) {
			return false;
		}

		if (!FileSystem::WriteInteger(file, ite->second.type)) {
			return false;
		}

		if (!FileSystem::WriteInteger(file, ite->second.parameter)) {
			return false;
		}
	}

	return true;
}

bool Serializer::SaveLRGotoTable(std::ofstream& file, const LRGotoTable &gotoTable) {
	FileSystem::WriteInteger(file, gotoTable.size());
	for (LRGotoTable::const_iterator ite = gotoTable.begin(); ite != gotoTable.end(); ++ite) {
		if (!FileSystem::WriteInteger(file, ite->first.first)) {
			return false;
		}

		if (!FileSystem::WriteString(file, ite->first.second->ToString())) {
			return false;
		}

		if (!FileSystem::WriteInteger(file, ite->second)) {
			return false;
		}
	}

	return true;
}

bool Serializer::LoadLRActionTable(std::ifstream& file, GrammarSymbolContainer& terminalSymbols, LRTable &table) {
	int count = 0;
	if (!FileSystem::ReadInteger(file, &count)) {
		return false;
	}

	std::string stext;
	for (int i = 0; i < count; ++i) {
		int from, actionType, actionParameter;
		if (!FileSystem::ReadInteger(file, &from)) {
			return false;
		}

		if (!FileSystem::ReadString(file, &stext)) {
			return false;
		}

		GrammarSymbolPtr symbol = terminalSymbols[stext];
		if (symbol == NativeSymbols::null) {
			Debug::LogError("invalid terminal symbol %s.", stext.c_str());
			return false;
		}

		if (!FileSystem::ReadInteger(file, &actionType)) {
			return false;
		}

		if (!FileSystem::ReadInteger(file, &actionParameter)) {
			return false;
		}

		LRAction action = { (LRActionType)actionType, actionParameter };
		table.actionTable_.insert(from, symbol, action);
	}

	return true;
}

bool Serializer::LoadLRGotoTable(std::ifstream& file, GrammarSymbolContainer& nonterminalSymbols, LRTable &table) {
	int count = 0;
	if (!FileSystem::ReadInteger(file, &count)) {
		return false;
	}

	std::string stext;
	for (int i = 0; i < count; ++i) {
		int from, to;
		if (!FileSystem::ReadInteger(file, &from)) {
			return false;
		}

		if (!FileSystem::ReadString(file, &stext)) {
			return false;
		}

		GrammarSymbolPtr symbol = nonterminalSymbols[stext];
		if (symbol == NativeSymbols::null) {
			Debug::LogError("invalid terminal symbol %s.", stext.c_str());
			return false;
		}

		if (!FileSystem::ReadInteger(file, &to)) {
			return false;
		}

		table.gotoTable_.insert(from, symbol, to);
	}

	return true;
}
