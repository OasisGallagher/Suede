#pragma once
#include <map>

#include "syntaxtree.h"
#include "glefdefines.h"

class SUEDE_API GLEF {
public:
	static void Initialize();

public:
	static bool Parse(const char* path, SyntaxTree& tree);
	static float Evaluate(const char* expression, const std::map<std::string, float>* variables);

private:
	GLEF();
};
