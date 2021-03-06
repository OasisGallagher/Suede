#include <fstream>

#include "language.h"

#include "debug/debug.h"
#include "tools/string.h"
#include "grammarsymbol.h"
#include "os/filesystem.h"

#include "scanner/scanner.h"

#include "parser/parser.h"
#include "parser/syntaxer.h"
#include "parser/lrparser.h"

typedef std::pair<std::string, std::string> ProductionText;

struct GrammarText {
	std::string lhs;
	typedef std::vector<ProductionText> ProductionTextContainer;
	ProductionTextContainer productions;

	void Clear() {
		lhs.clear();
		productions.clear();
	}

	bool Empty() const { return lhs.empty(); }
};

typedef std::vector<GrammarText> GrammarTextContainer;

class GrammarReader {
public:
	GrammarReader(const char* source);

public:
	const GrammarTextContainer& GetGrammars() const;

private:
	void ReadGrammars();
	const char* SplitGrammar(const char*& text);

private:
	const char* source_;
	GrammarTextContainer grammars_;
};

GrammarReader::GrammarReader(const char* source) : source_(source) {
	ReadGrammars();
}

const GrammarTextContainer& GrammarReader::GetGrammars() const {
	return grammars_;
}

const char* GrammarReader::SplitGrammar(const char*& text) {
	text += strspn(text, ":\t\n ");
	if (*text == '|') {
		++text;
	}

	return std::find(text, text + strlen(text), '\t');
}

void GrammarReader::ReadGrammars() {
	GrammarText g;
	std::vector<std::string> lines;
	String::Split(lines, source_, '\n');
	for (int i = 0;  i < lines.size(); ++i) {
		std::string line = lines[i];

		if (String::IsBlankText(line.c_str(), nullptr)) {
			if (!g.Empty()) {
				grammars_.push_back(g);
				g.Clear();
			}

			continue;
		}

		if (g.Empty()) {
			g.lhs = String::Trim(line);
			continue;
		}

		const char* ptr = line.c_str();
		const char* tabpos = SplitGrammar(ptr);
		if (*tabpos == 0) {
			Debug::LogError("missing \\t between production and action at line %d.", (i + 1));
			break;
		}

		g.productions.push_back(std::make_pair(String::Trim(std::string(ptr, tabpos)), String::Trim(std::string(tabpos))));
	}

	if (!g.Empty()) {
		grammars_.push_back(g);
	}
}


static GrammarSymbolPtr CreateSymbol(Environment* env, const std::string& text) {
	GrammarSymbolContainer* target = nullptr;
	if (GrammarSymbol::IsTerminal(text)) {
		target = &env->terminalSymbols;
	}
	else {
		target = &env->nonterminalSymbols;
	}

	GrammarSymbolContainer::iterator ite = target->find(text);
	GrammarSymbolPtr ans = nullptr;

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
	scanner_ = new SourceScanner;
}

Language::~Language() {
	delete env_;
	delete scanner_;
	delete syntaxer_;
}

bool Language::Parse(SyntaxTree* tree, const std::string& path) {
	return scanner_->Open(path) && syntaxer_->ParseSyntax(tree, scanner_);
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

bool Language::BuildSyntaxer(const char* grammars) {
	SetupEnvironment(grammars);
	LRParser parser;
	return parser.Setup(*syntaxer_, env_);
}

bool Language::LoadSyntaxer(const char* savePath) {
	std::ifstream file(savePath, std::ios::binary);
	if (!file) {
		Debug::LogError("faild to load syntaxer from \"%s\"", savePath);
		return false;
	}

	if (!env_->Load(file)) {
		return false;
	}

	SyntaxerSetupParameter p = { env_ };
	syntaxer_->Setup(p);

	return syntaxer_->Load(file);
}

bool Language::SaveSyntaxer(const char* savePath) {
	std::ofstream file(savePath, std::ios::binary);
	if (!file) {
		Debug::LogError("failed to save syntaxer to %s.", savePath);
		return false;
	}

	env_->Save(file);
	syntaxer_->Save(file);
	file.close();
	return true;
}

std::string Language::ToString() const {
	return syntaxer_->ToString();
}
