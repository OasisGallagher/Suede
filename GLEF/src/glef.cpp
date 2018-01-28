#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "glef.h"
#include "os/os.h"
#include "language.h"
#include "debug/debug.h"
#include "glefgrammar.h"

static Language language;
const char* binary = "bin/GLEF.bin";

GLEF::GLEF() {
	time_t tp = OS::GetFileLastWriteTime("bin/GLEF.dll");
	time_t to = OS::GetFileLastWriteTime(binary);

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
	static GLEF instance_;
	return language.Parse(&tree, path);
}
