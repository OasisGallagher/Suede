#pragma once
#include "compiler_defines.h"

enum ScannerTokenType {
	ScannerTokenError,
	ScannerTokenEndOfFile,

	ScannerTokenIdentifier,
	ScannerTokenNumber,
	ScannerTokenString,

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
	char text[COMPOLIER_MAX_TOKEN_CHARACTERS];
};
