#pragma once
#include <map>
#include <vector>
#include "scanner/scanner.h"

class MathExpr {
public:
	typedef std::map<std::string, float> Variables;
	static float Eval(const char* expression, const Variables* variables);

private:
	static bool ShuntingYard(std::vector<std::string>& tokens, const char* expression);

	static float Calculate(float lhs, float rhs, ScannerTokenType type);
	static bool ToFloat(float& f, const std::string& token, const Variables* variables);
	static float Calculate(std::vector<std::string>& tokens, const Variables* variables);
};
