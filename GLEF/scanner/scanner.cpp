#include <cassert>
#include <sstream>
#include <algorithm>

#include "tokens.h"
#include "scanner.h"
#include "math/mathf.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "glefdefines.h"
#include "os/filesystem.h"

enum {
	StartState,
	DoneState,
	DecimalState,
	HexState,
	SingleState,
	StringState,
	StringState2,
	IdentifierState,
};

std::string TokenPosition::ToString() const {
	std::ostringstream oss;
	oss << lineno << ":" << linepos;
	return oss.str();
}

TextScanner::TextScanner() 
	: current_(nullptr), dest_(nullptr), leadingNegative_(false), lastTokenType_(ScannerTokenEndOfFile) {
}

TextScanner::~TextScanner() {
}

void TextScanner::SetText(const std::string& value) {
	lineBuffer_ = value;
	lastTokenType_ = ScannerTokenEndOfFile;

	start_ = current_ = lineBuffer_.c_str();
	dest_ = lineBuffer_.c_str() + lineBuffer_.length() + 1;
}

bool TextScanner::GetChar(int* ch) {
	if (current_ == nullptr || current_ == dest_) {
		return false;
	}

	*ch = *current_++;
	return true;
}

void TextScanner::UngetChar() {
	if (current_ == nullptr || current_ <= lineBuffer_.c_str()) {
		Debug::LogError("unget failed. invalid state");
		return;
	}

	--current_;
}

ScannerTokenType TextScanner::GetToken(std::string& token, int* pos) {
	return GetNextToken(token, pos);
}

ScannerTokenType TextScanner::GetNextToken(std::string& token, int* pos) {
	int ci = 0, ch = 0, state = StartState;
	bool savech = true, unget = false;

	std::string buffer;
	ScannerTokenType tokenType = ScannerTokenError;

	for (; state != DoneState;) {
		if (!GetChar(&ch)) {
			if (pos != nullptr) {
				*pos = -1;
			}

			return ScannerTokenEndOfFile;
		}

		savech = true;
		unget = false;

		switch (state) {
		case StartState:
			if (ch == ' ' || ch == '\t' || ch == 0) {
				savech = false;
			}
			else if ((!leadingNegative_ || (lastTokenType_ == ScannerTokenEndOfFile))
				&& (ch == '-' || ch == '+')) {
				state = DecimalState;
			}
			else if (String::IsDigit(ch)) {
				state = DecimalState;
			}
			else if (String::IsLetter(ch)) {
				state = IdentifierState;
			}
			else if (ch == '\'') {
				state = StringState;
				savech = false;
			}
			else if (ch == '"') {
				state = StringState2;
				savech = false;
			}
			else if (ch == '\n') {
				state = DoneState;
				tokenType = ScannerTokenNewline;
			}
			else {
				int low = 0, high = 0;

				for (; low < Tokens::Size() && Tokens::Text(low)[ci] != ch; ++low) {
				}

				for (high = low + 1; high < Tokens::Size() && Tokens::Text(high)[ci] == ch; ++high) {
				}

				if (high > Tokens::Size()) {
					Debug::LogError("can not find item leading with %c.", (char)ch);
				}

				++ci;
				if (high - low == 1 && strlen(Tokens::Text(low)) == ci) {
					state = DoneState;
					tokenType = Tokens::Type(low);
				}
				else {
					state = Mathf::MakeDword(low, high);
				}

				savech = true;
			}
			break;

		case StringState:
			if (ch == '\'') {
				tokenType = ScannerTokenString;
				savech = false;
				state = DoneState;
			}
			break;

		case StringState2:
			if (ch == '"') {
				tokenType = ScannerTokenString;
				savech = false;
				state = DoneState;
			}
			break;

		case IdentifierState:
			if (!String::IsDigit(ch) && !String::IsLetter(ch)) {
				state = DoneState;
				tokenType = ParseIdentifierType(buffer);
				unget = true;
				savech = false;
			}
			break;

		case SingleState:
			if (ch == 'f' || ch == 'F') {
				state = DoneState;
				tokenType = ScannerTokenSingle;
			}
			else if (!String::IsDigit(ch)) {
				state = DoneState;
				tokenType = ScannerTokenSingle;
				unget = true;
				savech = false;
			}
			break;

		case HexState:
			if (!String::IsDigit(ch)
				&& !((ch <= 'f' && ch >= 'a') || (ch <= 'F' && ch >= 'A'))) {
				state = DoneState;
				tokenType = ScannerTokenInteger;
				unget = true;
				savech = false;
			}
			break;

		case DecimalState:
			if (!String::IsDigit(ch)) {
				if (ch == '.') {
					state = SingleState;
				}
				else if (ch == 'f' || ch == 'F') {
					state = DoneState;
					tokenType = ScannerTokenSingle;
				}
				else if (buffer.length() == 1 && ch == 'x') {
					state = HexState;
				}
				else {
					state = DoneState;
					tokenType = ScannerTokenInteger;
					unget = true;
					savech = false;
				}
			}
			break;

		default:
			int low = Mathf::Loword(state), high = Mathf::Highword(state);
			
			savech = false;
			if (ch == 0) {
				if (strlen(Tokens::Text(low)) != ci) {
					Debug::LogError("invalid symbol.");
					break;
				}

				state = DoneState;
				tokenType = Tokens::Type(low);
			}
			else {
				int nl = low;
				for (; nl < high && Tokens::Text(nl)[ci] != ch; ++nl) {
				}

				if (nl < high) {
					low = nl;
				}

				int nh = low + 1;
				for (; nh < high && Tokens::Text(nh)[ci] == ch; ++nh) {
				}
				high = nh;

				savech = (Tokens::Text(low)[ci] == ch);
				unget = !savech;

				if (savech) {
					++ci;
				}

				if (high - low == 1 && strlen(Tokens::Text(low)) == ci) {
					state = DoneState;
					tokenType = Tokens::Type(low);
				}
				else {
					state = Mathf::MakeDword(low, high);
				}
			}

			break;
		}

		if (unget) {
			UngetChar();
		}

		if (savech) {
			buffer.push_back((char)ch);
		}
	}

	token = buffer;

	if (pos != nullptr) {
		*pos = int(1 + current_ - start_ - buffer.length());
	}

	lastTokenType_ = tokenType;
	return tokenType;
}

ScannerTokenType TextScanner::ParseIdentifierType(const std::string& buffer) {
	if (buffer == GLEF_GLSL_CODE_BEGIN) { return ScannerTokenCode; }
	if (buffer == GLEF_TRUE_TEXT || buffer == GLEF_FALSE_TEXT) { return ScannerTokenBoolean; }
	return ScannerTokenIdentifier;
}

SourceScanner::SourceScanner() {
}

SourceScanner::~SourceScanner() {
}

bool SourceScanner::Open(const std::string& path) {
	index_ = 0;
	text_.clear();

	if (!FileSystem::ReadAllText(path, text_)) {
		return false;
	}

	lines_.clear();
	String::Split(lines_, text_, '\n');

	return true;
}

bool SourceScanner::GetToken(ScannerToken* token, TokenPosition* pos) {
	std::string line, buffer;
	ScannerTokenType tokenType = textScanner_.GetToken(buffer, &pos->linepos);
	for (; tokenType == ScannerTokenCode || tokenType == ScannerTokenEndOfFile
		|| tokenType == ScannerTokenComment; ) {
		if (tokenType == ScannerTokenCode) {
			break;
		}

		if (index_ >= lines_.size()) {
			tokenType = ScannerTokenEndOfFile;
			break;
		}

		line = lines_[index_++];

		const char* nonBlankPtr = nullptr;
		if (String::IsBlankText(line.c_str(), &nonBlankPtr)) {
			continue;
		}

		textScanner_.SetText(line);
		tokenType = textScanner_.GetToken(buffer, &pos->linepos);
	}

	pos->lineno = index_;

	if (tokenType == ScannerTokenError) {
		return false;
	}

	if (tokenType == ScannerTokenCode) {
		textScanner_.Discard();
		return ReadCode(token);
	}

	token->tokenType = tokenType;
	token->tokenText = buffer;

	return true;
}

bool SourceScanner::ReadCode(ScannerToken* token) {
	std::string line, code;

	for (bool first = true; index_ < lines_.size();) {
		line = lines_[index_++];

		std::string str = String::Trim(line);
		if (str == GLEF_GLSL_CODE_END) {
			token->tokenType = ScannerTokenCode;
			token->tokenText = code;
			return true;
		}

		if (!first) {
			code += '\n';
		}

		code += line;
		first = false;
	}

	Debug::LogError("missing %s.", GLEF_GLSL_CODE_END);

	token->tokenType = ScannerTokenError;
	return false;
}
