#pragma once
#include "syntaxtree.h"
#include "glefdefines.h"

class GLEF_API GLEF {
public:
	static bool Parse(const char* path, SyntaxTree& tree);

private:
	GLEF();
	~GLEF();
};
