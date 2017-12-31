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

class LiteralTable : public PtrMap<std::string, Literal> { };

class CodeTable : public PtrMap<std::string, Code> { };

std::string Sym::ToString() const {
	return value_;
}

void Integer::SetText(const std::string& text) {
	int integer = 0;
	if (!String::ToInteger(text.c_str(), &integer)) {
		Debug::LogError("invalid integer %s.", text.c_str());
	}

	value_ = integer;
}

std::string Integer::ToString() const {
	return std::to_string(value_);
}

std::string Literal::ToString() const {
	return value_;
}

std::string Code::ToString() const {
	return value_;
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
}

Syntaxer::~Syntaxer() {
	delete stack_;
	delete symTable_;
	delete codeTable_;
	delete literalTable_;
	delete integerTable_;
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

bool Syntaxer::ParseSyntax(SyntaxTree* tree, FileScanner* fileScanner) {
	SyntaxNode* root = nullptr;
	if (!CreateSyntaxTree(root, fileScanner)) {
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

bool Syntaxer::CreateSyntaxTree(SyntaxNode*& root, FileScanner* fileScanner) {
	TokenPosition position = { 0 };

	stack_->push(0, nullptr, NativeSymbols::zero);
	LRAction action = { LRActionShift };

	void* addr = nullptr;
	GrammarSymbol symbol = nullptr;

	do {
		if (action.type == LRActionShift && !(symbol = ParseNextSymbol(position, addr, fileScanner))) {
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

GrammarSymbol Syntaxer::ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner) {
	ScannerToken token;
	GrammarSymbol answer = NativeSymbols::null;

	if (fileScanner->GetToken(&token, &position)) {
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
