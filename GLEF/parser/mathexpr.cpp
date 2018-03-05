#include <map>

#include "mathexpr.h"
#include "debug/debug.h"
#include "tools/string.h"

static float NaN = nanf("");
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
	scanner.SetLeadingNegative(true);

	std::string token;
	bool positive = true;
	std::vector<ScannerTokenType> operators;

	for (ScannerTokenType type; (type = scanner.GetToken(token, false)) != ScannerTokenEndOfFile;) {
		switch (type) {
			case ScannerTokenSingle:
			case ScannerTokenInteger:
				if (token == "-") {
					positive = !positive;
				}
				else {
					if (!positive) {
						token = "-" + token;
						positive = true;
					}

					tokens.push_back(token);
				}
				break;

			case ScannerTokenIdentifier:
				if (!positive) {
					token = "-" + token;
					positive = true;
				}

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
	std::vector<float> stack;

	for (int i = 0; i < tokens.size(); ++i) {
		float f;
		if (tokens[i][0] == '#') {
			ScannerTokenType op = (ScannerTokenType)String::ToInteger(tokens[i].c_str() + 1);
			f = Calculate(*++stack.rbegin(), stack.back(), op);
			stack.pop_back();
			stack.pop_back();
		}
		else {
			const char* ptr = tokens[i].c_str();
			if (tokens[i][0] == '-') { ++ptr; }
			if (!ToFloat(f, ptr, variables)) {
				return false;
			}

			if (tokens[i][0] == '-') { f = -f; }
		}

		stack.push_back(f);
	}

	if (stack.size() != 1) {
		Debug::LogError("failed to calculate expression.");
		return NaN;
	}

	return stack.back();
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
