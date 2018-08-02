#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "glef.h"
#include "debug/debug.h"
#include "os/filesystem.h"
#include "parser/mathexpr.h"
#include "grammar/language.h"
#include "grammar/glefgrammar.h"

static Language language;
const char* binary = "bin/GLEF.bin";

GLEF::GLEF() {
	time_t tp = FileSystem::GetFileLastWriteTime("bin/GLEF.dll");
	time_t to = FileSystem::GetFileLastWriteTime(binary);

	if (tp > to) {
		language.BuildSyntaxer(GLEFGrammar);
		language.SaveSyntaxer(binary);
	}
	else {
		language.LoadSyntaxer(binary);
	}
}

bool GLEF::Parse(const char* path, SyntaxTree& tree) {
	return language.Parse(&tree, path);
}

float GLEF::Evaluate(const char* expression, const std::map<std::string, float>* variables) {
	return MathExpr::Eval(expression, variables);
}
