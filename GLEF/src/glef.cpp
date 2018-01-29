#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "glef.h"
#include "language.h"
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
	static GLEF instance_;
	return language.Parse(&tree, path);
}
