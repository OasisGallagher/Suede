#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "glef.h"
#include "language.h"
#include "mathexpr.h"
#include "debug/debug.h"
#include "glefgrammar.h"
#include "os/filesystem.h"

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

GLEF::~GLEF() {
}

bool GLEF::Parse(const char* path, SyntaxTree& tree) {
	std::map<std::string, float> variables;
	variables["A"] = 1;
	variables["B"] = 2;
	variables["C"] = 3;
	variables["D"] = 4;

	float f = Evaluate("A+B*C-D", &variables);
	static GLEF instance_;
	return language.Parse(&tree, path);
}

float GLEF::Evaluate(const char* expression, const std::map<std::string, float>* variables) {
	return MathExpr::Eval(expression, variables);
}
