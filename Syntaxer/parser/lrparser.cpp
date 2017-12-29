#include <sstream>

#include "lalr.h"
#include "action.h"
#include "matrix.h"
#include "scanner.h"
#include "syntaxer.h"
#include "lrtable.h"
#include "lrparser.h"
#include "syntaxtree.h"

LRParser::LRParser() {
	impl_ = new LALR;
	lrTable_ = new LRTable();
}

LRParser::~LRParser() {
	delete impl_;
	delete lrTable_;
}

bool LRParser::ParseGrammars(Syntaxer& syntaxer, Environment* env) {
	impl_->Setup(env, &firstSetContainer_);

	if (!impl_->Parse(lrTable_->actionTable_, lrTable_->gotoTable_)) {
		return false;
	}

	SyntaxerSetupParameter p = { env_, *lrTable_ };
	syntaxer.Setup(p);
	return true;
}

void LRParser::Clear() {
	Parser::Clear();
}

std::string LRParser::ToString() const {
	std::ostringstream oss;

	//oss << Parser::ToString();
	//oss << "\n\n";

	//oss << impl_->ToString();

	//oss << firstSetContainer_.ToString();
	//oss << "\n\n";
	//oss << lrTable_->ToString();

	return oss.str();
}
