#include <map>

#include "mathexpr.h"
#include "debug/debug.h"
#include "tools/string.h"

#define NaN nanf("")
#define StrOperator(op)		"#" + std::to_string(op)
#define Precedence(type)	int((type) == ScannerTokenMultiply || (type) == ScannerTokenDivide)

float MathExpr::Eval(const char* expression, const Variables* variables) {
	std::vector<std::string> tokens;
	if (!ShuntingYard(tokens, expression)) {
		return NaN;
	}

	return Calculate(tokens, variables);
}

bool MathExpr::ShuntingYard(std::vector<std::string>& tokens, const char* expression) {
	TextScanner scanner;
	scanner.SetText(expression);

	std::string token;
	std::vector<ScannerTokenType> operators;

	for (ScannerTokenType type; (type = scanner.GetToken(token, false)) != ScannerTokenEndOfFile;) {
		switch (type) {
			case ScannerTokenSingle:
			case ScannerTokenInteger:
			case ScannerTokenIdentifier:
				tokens.push_back(token);
				break;

			case ScannerTokenPlus:
			case ScannerTokenMinus:
			case ScannerTokenMultiply:
			case ScannerTokenDivide:
				for (; !operators.empty()
					&& operators.back() != ScannerTokenLeftParenthesis
					&& Precedence(operators.back()) >= Precedence(type); operators.pop_back()) {
					tokens.push_back(StrOperator(operators.back()));
				}

				operators.push_back(type);
				break;

			case ScannerTokenLeftParenthesis:
				operators.push_back(type);
				break;

			case ScannerTokenRightParenthesis:
				for (; operators.back() != ScannerTokenLeftParenthesis; operators.pop_back()) {
					tokens.push_back(StrOperator(operators.back()));
				}

				operators.pop_back();
				break;

			default:
				Debug::LogError("invalid token %s.", token.c_str());
				type = ScannerTokenError;
				break;
		}

		if (type == ScannerTokenError) {
			return false;
		}
	}

	for (; !operators.empty(); operators.pop_back()) {
		tokens.push_back("#" + std::to_string(operators.back()));
	}

	return true;
}

float MathExpr::Calculate(std::vector<std::string>& tokens, const Variables* variables) {
	float rhs = NaN;
	for (int i = 0; i < tokens.size(); ++i) {
		if (tokens[i][0] == '#') {
			ScannerTokenType op = (ScannerTokenType)String::ToInteger(tokens[i].c_str() + 1);
			float lhs;
			if (isnan(rhs) && (!ToFloat(lhs, tokens[i - 2], variables) || !ToFloat(rhs, tokens[i - 1], variables))) {
				return false;
			}

			if (!isnan(rhs) && !ToFloat(lhs, tokens[i - 1], variables)) {
				return false;
			}

			rhs = Calculate(lhs, rhs, op);
		}
	}

	return rhs;
}

float MathExpr::Calculate(float lhs, float rhs, ScannerTokenType type) {
	switch (type) {
		case ScannerTokenPlus: return lhs + rhs;
		case ScannerTokenMinus: return lhs - rhs;
		case ScannerTokenMultiply: return lhs * rhs;
		case ScannerTokenDivide: return lhs / rhs;
	}

	return NaN;
}

bool MathExpr::ToFloat(float& f, const std::string& token, const Variables* variables) {
	if (String::ToFloat(token, &f)) {
		return true;
	}

	Variables::const_iterator ite;
	if (variables == nullptr || (ite = variables->find(token)) == variables->end()) {
		Debug::LogError("undefined variable %s.", token.c_str());
		return false;
	}

	f = ite->second;
	return true;
}
