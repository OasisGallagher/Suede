#pragma once
#include <string>
#include "glefdefines.h"

enum ScannerTokenType {
	ScannerTokenError,
	ScannerTokenEndOfFile,

	ScannerTokenIdentifier,
	ScannerTokenInteger,
	ScannerTokenString,
	ScannerTokenCode,

	ScannerTokenNewline,

	ScannerTokenPlus,
	ScannerTokenMinus,
	ScannerTokenMultiply,
	ScannerTokenDivide,
	ScannerTokenMod,
	
	ScannerTokenPlusEqual,
	ScannerTokenMinusEqual,
	ScannerTokenMultiplyEqual,
	ScannerTokenDivideEqual,
	ScannerTokenModEqual,

	ScannerTokenSelfIncrement,
	ScannerTokenSelfDecrement,

	ScannerTokenShiftLeft,
	ScannerTokenShiftRight,

	ScannerTokenShiftLeftEqual,
	ScannerTokenShiftRightEqual,

	ScannerTokenBitwiseAnd,
	ScannerTokenBitwiseOr,
	ScannerTokenBitwiseXor,
	ScannerTokenBitwiseNot,

	ScannerTokenBitwiseAndEqual,
	ScannerTokenBitwiseOrEqual,
	ScannerTokenBitwiseXorEqual,

	ScannerTokenLess,
	ScannerTokenGreater,
	ScannerTokenAssign,

	ScannerTokenOr,
	ScannerTokenAnd,
	ScannerTokenXor,

	ScannerTokenLessEqual,
	ScannerTokenGreaterEqual,
	ScannerTokenEqual,
	ScannerTokenNotEqual,

	ScannerTokenLeftBrace,
	ScannerTokenRightBrace,

	ScannerTokenLeftParenthesis,
	ScannerTokenRightParenthesis,
	
	ScannerTokenLeftSquareBracket,
	ScannerTokenRightSquareBracket,
	
	ScannerTokenQuestionmark,
	ScannerTokenExclamation,
	ScannerTokenSemicolon,
	ScannerTokenColon,
	ScannerTokenComma,
	ScannerTokenDot,
};

struct ScannerToken {
	ScannerTokenType tokenType;
	std::string tokenText;
};
