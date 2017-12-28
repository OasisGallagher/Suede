#include "os/os.h"
#include "reader.h"
#include "parser.h"
#include "scanner.h"
#include "language.h"
#include "syntaxer.h"
#include "lr_parser.h"
#include "debug/debug.h"
#include "grammar_symbol.h"

static GrammarSymbol CreateSymbol(Environment* env, const std::string& text) {
	GrammarSymbolContainer* target = nullptr;
	if (Utility::IsTerminal(text)) {
		target = &env->terminalSymbols;
	}
	else {
		target = &env->nonterminalSymbols;
	}

	GrammarSymbolContainer::iterator ite = target->find(text);
	GrammarSymbol ans = nullptr;

	if (ite == target->end()) {
		ans = SymbolFactory::Create(text);
		target->insert(std::make_pair(text, ans));
	}
	else {
		ans = ite->second;
	}

	return ans;
}

static bool ParseProduction(Environment* env, TextScanner* textScanner, SymbolVector& symbols) {
	std::string token;
	
	for (ScannerTokenType tokenType; (tokenType = textScanner->GetToken(token)) != ScannerTokenEndOfFile;) {
		symbols.push_back(CreateSymbol(env, token));
	}

	if (symbols.front() == NativeSymbols::epsilon && symbols.size() != 1) {
		Debug::LogError("invalid epsilon usage.");
		return false;
	}

	return true;
}

Language::Language() {
	env_ = new Environment;
	syntaxer_ = new Syntaxer;
}

Language::~Language() {
	delete env_;
	delete syntaxer_;
}

void Language::Setup(const char* grammars, const char* savePath) {
	time_t tp = OS::GetFileLastWriteTime("bin/Compiler.dll");
	time_t to = OS::GetFileLastWriteTime(savePath);
	if (tp > to) {
		Debug::Log("build parser");
		BuildSyntaxer(grammars);
		SaveSyntaxer(savePath);
	}
	else {
		Debug::Log("load parser");
		LoadSyntaxer(savePath);
	}
}

bool Language::Parse(SyntaxTree* tree, const std::string& path) {
	FileScanner scanner(path.c_str());
	return syntaxer_->ParseSyntax(tree, &scanner);
}

bool Language::SetupEnvironment(const char* grammars) {
	NativeSymbols::Copy(env_->terminalSymbols, env_->nonterminalSymbols);

	if (!ParseProductions(grammars)) {
		return false;
	}

	if (env_->grammars.empty()) {
		Debug::LogError("grammar container is empty");
		return false;
	}

	if (env_->grammars.front()->GetLhs() != NativeSymbols::program) {
		Debug::LogError("invalid grammar. missing \"Program\".");
		return false;
	}

	return true;
}

bool Language::ParseProductions(const char* grammars) {
	TextScanner textScanner;
	GrammarReader reader(grammars);
	const GrammarTextContainer& cont = reader.GetGrammars();
	for (GrammarTextContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		const GrammarText& g = *ite;

		Grammar* grammar = new Grammar(CreateSymbol(env_, g.lhs));
		env_->grammars.push_back(grammar);

		SymbolVector symbols;

		for (GrammarText::ProductionTextContainer::const_iterator ite2 = g.productions.begin(); ite2 != g.productions.end(); ++ite2) {
			const ProductionText& pr = *ite2;
			textScanner.SetText(pr.first.c_str());

			if (!ParseProduction(env_, &textScanner, symbols)) {
				return false;
			}

			grammar->AddCondinate(pr.second, symbols);
			symbols.clear();
		}
	}

	return true;
}

void Language::BuildSyntaxer(const char* grammars) {
	SetupEnvironment(grammars);
	LRParser parser;
	parser.Setup(*syntaxer_, env_);
}

void Language::LoadSyntaxer(const char* savePath) {
	std::ifstream file(savePath, std::ios::binary);
	env_->Load(file);
	SyntaxerSetupParameter p = { env_ };
	syntaxer_->Setup(p);
	syntaxer_->Load(file);
}

void Language::SaveSyntaxer(const char* savePath) {
	std::ofstream file(savePath, std::ios::binary);
	env_->Save(file);
	syntaxer_->Save(file);
}

std::string Language::ToString() const {
	return syntaxer_->ToString();
}
