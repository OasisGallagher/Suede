#pragma once
#include <set>
#include <map>
#include <vector>

#include "memory/refptr.h"

enum GrammarSymbolType {
	GrammarSymbolTerminal,
	GrammarSymbolNonterminal,
};

struct ScannerToken;

class GrammarSymbol : public intrusive_ref_counter {
public:
	friend class SymbolFactory;

	const std::string& ToString() { return text_; }

	static bool IsTerminal(const std::string& token) {
		return token.front() != '$';
	}

public:
	virtual GrammarSymbolType SymbolType() const = 0;

protected:
	GrammarSymbol(const std::string& text) : text_(text) { }
	~GrammarSymbol() { }

protected:
	std::string text_;
};

class TerminalSymbol : public GrammarSymbol {
public:
	TerminalSymbol(const std::string& text) : GrammarSymbol(text) { }

public:
	virtual GrammarSymbolType SymbolType() const { return GrammarSymbolTerminal; }
};

class NonterminalSymbol : public GrammarSymbol {
public:
	NonterminalSymbol(const std::string& text) : GrammarSymbol(text) { }

public:
	virtual GrammarSymbolType SymbolType() const { return GrammarSymbolNonterminal; }
};

typedef ref_ptr<GrammarSymbol> GrammarSymbolPtr;

class SymbolFactory {
public:
	static GrammarSymbolPtr Create(const std::string& text);
};

typedef std::vector<GrammarSymbolPtr> SymbolVector;

class GrammarSymbolContainer : public std::map<std::string, GrammarSymbolPtr> {
public:
	std::string ToString() const;
};

class NativeSymbols {
public:
	static bool IsNative(const GrammarSymbolPtr& symbol);
	static void Copy(GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols);
	
public:
	static GrammarSymbolPtr null;
	static GrammarSymbolPtr zero;
	static GrammarSymbolPtr code;
	static GrammarSymbolPtr integer;
	static GrammarSymbolPtr single;
	static GrammarSymbolPtr boolean;
	static GrammarSymbolPtr string;
	static GrammarSymbolPtr unknown;
	static GrammarSymbolPtr epsilon;
	static GrammarSymbolPtr program;
	static GrammarSymbolPtr identifier;
};

typedef std::set<GrammarSymbolPtr> GrammarSymbolSet;

class GrammarSymbolSetTable : public std::map<GrammarSymbolPtr, GrammarSymbolSet> {
public:
	std::string ToString() const;
};

class FirstSetTable : public GrammarSymbolSetTable {
public:
	void GetFirstSet(GrammarSymbolSet& answer, SymbolVector::iterator first, SymbolVector::iterator last);
};
