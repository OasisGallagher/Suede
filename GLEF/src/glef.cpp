#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "glef.h"
#include "parser/mathexpr.h"
#include "grammar/language.h"
#include "grammar/glefgrammar.h"

static Language language;

bool RebuildGLEF(const char* binpath) {
	return language.BuildSyntaxer(GLEFGrammar) && language.SaveSyntaxer(binpath);
}

bool GLEF::Load(const char* binpath) {
	return language.LoadSyntaxer(binpath);
}

bool GLEF::Parse(const char* path, SyntaxTree& tree) {
	return language.Parse(&tree, path);
}

float GLEF::Evaluate(const char* expression, const std::map<std::string, float>* variables) {
	return MathExpr::Eval(expression, variables);
}
