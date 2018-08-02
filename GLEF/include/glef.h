#pragma once
#include <map>

#include "syntaxtree.h"
#include "glefdefines.h"
#include "tools/singleton.h"

class SUEDE_API GLEF : public Singleton<GLEF> {
	friend class Singleton<GLEF>;

public:
	bool Parse(const char* path, SyntaxTree& tree);
	float Evaluate(const char* expression, const std::map<std::string, float>* variables);

private:
	GLEF();
};
