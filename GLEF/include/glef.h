#pragma once
#include <map>

#include "syntaxtree.h"
#include "glefdefines.h"
#include "tools/singleton.h"

extern "C" {
	SUEDE_API bool RebuildGLEF(const char* binpath);
}

class SUEDE_API GLEF : public singleton<GLEF> {
	friend class singleton<GLEF>;

public:
	bool Load(const char* binpath);

	bool Parse(const char* path, SyntaxTree& tree);
	float Evaluate(const char* expression, const std::map<std::string, float>* variables);

private:
	GLEF() {}
};
