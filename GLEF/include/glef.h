#pragma once
#include <map>

#include "syntaxtree.h"
#include "glefdefines.h"

class GLEF_API GLEF {
public:
	static bool Parse(const char* path, SyntaxTree& tree);
	static float Evaluate(const char* expression, const std::map<std::string, float>* variables);

private:
	GLEF();
	~GLEF();
};
