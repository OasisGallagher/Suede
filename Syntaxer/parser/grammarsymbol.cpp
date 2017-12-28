#include <sstream>

#include "debug/debug.h"
#include "tokendefine.h"
#include "grammarsymbol.h"

GrammarSymbol NativeSymbols::null = nullptr;
GrammarSymbol NativeSymbols::zero = new TerminalSymbol("zero");
GrammarSymbol NativeSymbols::code = new TerminalSymbol("code");
GrammarSymbol NativeSymbols::unknown = new TerminalSymbol("#");
GrammarSymbol NativeSymbols::integer = new TerminalSymbol("integer");
GrammarSymbol NativeSymbols::string = new TerminalSymbol("string");
GrammarSymbol NativeSymbols::epsilon = new TerminalSymbol("epsilon");
GrammarSymbol NativeSymbols::identifier = new TerminalSymbol("identifier");
GrammarSymbol NativeSymbols::program = new NonterminalSymbol("$Program");

std::string GrammarSymbolContainer::ToString() const {
	std::ostringstream oss;
	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = " ";
		oss << ite->second.ToString();
	}

	return oss.str();
}

std::string GrammarSymbolSetTable::ToString() const {
	std::ostringstream oss;

	const char* newline = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		if (ite->first.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		oss << newline;
		newline = "\n";

		oss.width(32);
		oss.setf(std::ios::left);

		oss << ite->first.ToString();
		oss << "{ ";

		const char* seperator = "";
		for (GrammarSymbolSet::const_iterator ite2 = ite->second.begin(); ite2 != ite->second.end(); ++ite2) {
			oss << seperator;
			seperator = " ";
			oss << ite2->ToString();
		}

		oss << " }";
	}

	return oss.str();
}

void FirstSetTable::GetFirstSet(GrammarSymbolSet& answer, SymbolVector::iterator first, SymbolVector::iterator last) {
	for (; first != last; ++first) {
		iterator pos = find(*first);
		if (pos == end()) {
			if (first->SymbolType() != GrammarSymbolTerminal) {
				Debug::LogError("invalid symbol %s.", first->ToString().c_str());
				break;
			}

			answer.insert(*first); 
			break;
		}

		GrammarSymbolSet& firstSet = pos->second;
		bool hasEpsilon = false;
		for (GrammarSymbolSet::iterator ite = firstSet.begin(); ite != firstSet.end(); ++ite) {
			if (*ite != NativeSymbols::epsilon) {
				answer.insert(*ite);
			}
			else {
				hasEpsilon = true;
			}
		}

		if (!hasEpsilon) {
			break;
		}
	}

	if (first == last) {
		answer.insert(NativeSymbols::epsilon);
	}
}

GrammarSymbol SymbolFactory::Create(const std::string& text) {
	if (Utility::IsTerminal(text)) {
		return new TerminalSymbol(text);
	}

	return new NonterminalSymbol(text);
}

void NativeSymbols::Copy(GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols) {
	terminalSymbols.insert(std::make_pair(zero.ToString(), zero));
	terminalSymbols.insert(std::make_pair(code.ToString(), code));
	terminalSymbols.insert(std::make_pair(integer.ToString(), integer));
	terminalSymbols.insert(std::make_pair(string.ToString(), string));
	terminalSymbols.insert(std::make_pair(epsilon.ToString(), epsilon));
	terminalSymbols.insert(std::make_pair(identifier.ToString(), identifier));

	nonterminalSymbols.insert(std::make_pair(program.ToString(), program));
}

bool NativeSymbols::IsNative(const GrammarSymbol& symbol) {
	return symbol == null
		|| symbol == zero
		|| symbol == code
		|| symbol == integer
		|| symbol == string
		|| symbol == unknown
		|| symbol == epsilon
		|| symbol == program
		|| symbol == identifier;
}
