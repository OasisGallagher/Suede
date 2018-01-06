#include "parser.h"
#include "action.h"
#include "scanner.h"
#include "syntaxer.h"
#include "lrtable.h"
#include "serializer.h"
#include "syntaxtree.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "containers/ptrmap.h"

class SymTable : public PtrMap<std::string, Sym> { };

class IntegerTable : public PtrMap<std::string, Integer> { };

class SingleTable : public PtrMap<std::string, Single> { };

class BooleanTable : public PtrMap<std::string, Boolean> {};

class LiteralTable : public PtrMap<std::string, Literal> { };

class CodeTable : public PtrMap<std::string, Code> { };

void Integer::SetText(const std::string& text) {
	if (!String::ToInteger(text.c_str(), nullptr)) {
		Debug::LogError("invalid integer %s.", text.c_str());
	}

	value_ = text;
}

void Single::SetText(const std::string& text) {
	if (!String::ToFloat(text.c_str(), nullptr)) {
		Debug::LogError("invalid float %s.", text.c_str());
	}

	value_ = text;
}

void Boolean::SetText(const std::string& text) {
	if (!String::ToBool(text.c_str(), nullptr)) {
		Debug::LogError("invalid bool %s.", text.c_str());
	}

	value_ = text;
}

struct SyntaxerStack {
	std::vector<int> states;
	std::vector<void*> values;
	std::vector<GrammarSymbol> symbols;

	void push(int state, void* value, const GrammarSymbol& symbol);
	void pop(int count);
	void clear();
};

Syntaxer::Syntaxer() {
	stack_ = new SyntaxerStack;
	symTable_ = new SymTable;
	codeTable_ = new CodeTable;
	literalTable_ = new LiteralTable;
	integerTable_ = new IntegerTable;
	singleTable_ = new SingleTable;
	booleanTable_ = new BooleanTable;
}

Syntaxer::~Syntaxer() {
	delete stack_;
	delete symTable_;
	delete codeTable_;
	delete literalTable_;
	delete integerTable_;
	delete singleTable_;
	delete booleanTable_;
}

void Syntaxer::Setup(const SyntaxerSetupParameter& p) {
	p_ = p;
}

bool Syntaxer::Load(std::ifstream& file) {
	return Serializer::LoadSyntaxer(file, p_);
}

bool Syntaxer::Save(std::ofstream& file) {
	return Serializer::SaveSyntaxer(file, p_);
}

bool Syntaxer::ParseSyntax(SyntaxTree* tree, SourceScanner* sourceScanner) {
	SyntaxNode* root = nullptr;
	if (!CreateSyntaxTree(root, sourceScanner)) {
		return false;
	}

	tree->SetRoot(root);

	//Debug::Log("Accepted");

	return true;
}

std::string Syntaxer::ToString() const {
	return p_.lrTable.ToString(p_.env->grammars);
}

void Syntaxer::Clear() {
	stack_->clear();
	symTable_->clear();
	codeTable_->clear();
	literalTable_->clear();
	integerTable_->clear();
	singleTable_->clear();
}

int Syntaxer::Reduce(int cpos) {
	Grammar* g = nullptr;
	const Condinate* cond = p_.env->grammars.GetTargetCondinate(cpos, &g);

	int length = cond->symbols.front() == NativeSymbols::epsilon ? 0 : cond->symbols.size();

	std::string log = ">> [R] `" + String::Concat(stack_->symbols.end() - length, stack_->symbols.end()) + "` to `" + g->GetLhs().ToString() + "`.";

	void* newValue = (cond->action != nullptr) ? cond->action->Invoke(stack_->values) : nullptr;

	stack_->pop(length);

	int nextState = p_.lrTable.GetGoto(stack_->states.back(), g->GetLhs());
	//Debug::Log("%s Goto state %d.", log.c_str(), nextState);

	if (nextState < 0) {
		Debug::LogError("empty goto item(%d, %s).", stack_->states.back(), g->GetLhs().ToString().c_str());
		return nextState;
	}

	stack_->push(nextState, newValue, g->GetLhs());
	return nextState;
}

bool Syntaxer::Error(const GrammarSymbol& symbol, const TokenPosition& position) {
	Debug::LogError("unexpected symbol %s at %s.", symbol.ToString().c_str(), position.ToString().c_str());
	return false;
}

void Syntaxer::Shift(int state, void* addr, const GrammarSymbol& symbol) {
	//Debug::Log(">> [S] `%s`. Goto state %d.", symbol.ToString(), state);
	stack_->push(state, addr, symbol);
}

bool Syntaxer::CreateSyntaxTree(SyntaxNode*& root, SourceScanner* sourceScanner) {
	TokenPosition position = { 0 };

	stack_->push(0, nullptr, NativeSymbols::zero);
	LRAction action = { LRActionShift };

	void* addr = nullptr;
	GrammarSymbol symbol = nullptr;

	do {
		if (action.type == LRActionShift && !(symbol = ParseNextSymbol(position, addr, sourceScanner))) {
			break;
		}

		action = p_.lrTable.GetAction(stack_->states.back(), symbol);

		if (action.type == LRActionError && !Error(symbol, position)) {
			break;
		}

		if (action.type == LRActionShift) {
			Shift(action.parameter, addr, symbol);
		}
		else if (action.type == LRActionReduce) {
			if (!Reduce(action.parameter)) {
				break;
			}
		}

	} while (action.type != LRActionAccept);

	if (action.type == LRActionAccept) {
		root = (SyntaxNode*)stack_->values.back();
	}
	else {
		CleanupOnFailure();
	}

	Clear();

	return action.type == LRActionAccept;
}

GrammarSymbol Syntaxer::FindSymbol(const ScannerToken& token, void*& addr) {
	addr = nullptr;

	GrammarSymbol answer = NativeSymbols::null;
	if (token.tokenType == ScannerTokenEndOfFile) {
		answer = NativeSymbols::zero;
	}
	else if (token.tokenType == ScannerTokenInteger) {
		answer = NativeSymbols::integer;
		// TODO: set ?
		IntegerTable::ib_pair p = integerTable_->insert(token.tokenText);
		addr = p.first->second;
		if (p.second) {
			p.first->second->SetText(token.tokenText);
		}
	}
	else if (token.tokenType == ScannerTokenSingle) {
		answer = NativeSymbols::single;
		SingleTable::ib_pair p = singleTable_->insert(token.tokenText);
		addr = p.first->second;
		if (p.second) {
			p.first->second->SetText(token.tokenText);
		}
	}
	else if (token.tokenType == ScannerTokenBoolean) {
		answer = NativeSymbols::boolean;
		BooleanTable::ib_pair p = booleanTable_->insert(token.tokenText);
		addr = p.first->second;
		if (p.second) {
			p.first->second->SetText(token.tokenText);
		}
	}
	else if (token.tokenType == ScannerTokenString) {
		answer = NativeSymbols::string;
		LiteralTable::ib_pair p = literalTable_->insert(token.tokenText);
		addr = p.first->second;
		if (p.second) {
			p.first->second->SetText(token.tokenText);
		}
	}
	else if (token.tokenType == ScannerTokenCode) {
		answer = NativeSymbols::code;
		CodeTable::ib_pair p = codeTable_->insert(token.tokenText);
		addr = p.first->second;
		if (p.second) {
			p.first->second->SetText(token.tokenText);
		}
	}
	else {
		GrammarSymbolContainer::const_iterator pos = p_.env->terminalSymbols.find(token.tokenText);
		if (pos != p_.env->terminalSymbols.end()) {
			answer = pos->second;
		}
		else {
			answer = NativeSymbols::identifier;
			SymTable::ib_pair p = symTable_->insert(token.tokenText);
			addr = p.first->second;
			if (p.second) {
				p.first->second->SetText(token.tokenText);
			}
		}
	}

	if (answer == NativeSymbols::null) {
		Debug::LogError("can not find symbol %s.", token.tokenText);
	}

	return answer;
}

GrammarSymbol Syntaxer::ParseNextSymbol(TokenPosition& position, void*& addr, SourceScanner* sourceScanner) {
	ScannerToken token;
	GrammarSymbol answer = NativeSymbols::null;

	if (sourceScanner->GetToken(&token, &position)) {
		answer = FindSymbol(token, addr);
	}

	if (!answer) {
		Debug::LogError("invalid token at %s.", position.ToString().c_str());
	}

	return answer;
}

void Syntaxer::CleanupOnFailure() {
	SyntaxTree tree;
	for (int i = 0; i < (int)stack_->symbols.size(); ++i) {
		if (stack_->symbols[i].SymbolType() == GrammarSymbolNonterminal) {
			tree.SetRoot((SyntaxNode*)stack_->values[i]);
			tree.Destroy();
		}
	}
}

void SyntaxerStack::push(int state, void* value, const GrammarSymbol& symbol) {
	states.push_back(state);
	values.push_back(value);
	symbols.push_back(symbol);
}

void SyntaxerStack::pop(int count) {
	states.erase(states.end() - count, states.end());
	values.erase(values.end() - count, values.end());
	symbols.erase(symbols.end() - count, symbols.end());
}

void SyntaxerStack::clear() {
	states.clear();
	values.clear();
	symbols.clear();
}
