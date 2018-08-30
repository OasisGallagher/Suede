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

bool GLEF::Load(const char* binpath, const char* dllpath) {
#ifdef _DEBUG
	time_t tp = FileSystem::GetFileLastWriteTime(dllpath);
	time_t to = FileSystem::GetFileLastWriteTime(binpath);

	if (tp > to) {
		return language.BuildSyntaxer(GLEFGrammar) && language.SaveSyntaxer(binpath);
	}

#endif

	return language.LoadSyntaxer(binpath);
}

bool GLEF::Parse(const char* path, SyntaxTree& tree) {
	return language.Parse(&tree, path);
}

float GLEF::Evaluate(const char* expression, const std::map<std::string, float>* variables) {
	return MathExpr::Eval(expression, variables);
}
