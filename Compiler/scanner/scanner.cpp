#include <cassert>
#include <sstream>
#include <algorithm>

#include "tokens.h"
#include "reader.h"
#include "scanner.h"
#include "utilities.h"
#include "debug/debug.h"
#include "compiler_defines.h"

enum {
	StartState,
	DoneState,
	NumberState,
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
	: current_(nullptr), dest_(nullptr) {
}

TextScanner::~TextScanner() {
}

void TextScanner::SetText(const std::string& value) {
	lineBuffer_ = value;

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
	int ci = 0, bi = 0, ch = 0, state = StartState;
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
			else if ( ch == '@') {
				savech = false;
				state = DoneState;
				tokenType = ScannerTokenCode;
			}
			else if (Utility::IsDigit(ch)) {
				state = NumberState;
			}
			else if (Utility::IsLetter(ch)) {
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
					state = Utility::MakeDword(low, high);
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
			if (!Utility::IsDigit(ch) && !Utility::IsLetter(ch)) {
				state = DoneState;
				tokenType = ScannerTokenIdentifier;
				unget = true;
				savech = false;
			}
			break;

		case NumberState:
			if (!Utility::IsDigit(ch)) {
				state = DoneState;
				tokenType = ScannerTokenNumber;
				unget = true;
				savech = false;
			}
			break;

		default:
			int low = Utility::Loword(state), high = Utility::Highword(state);
			
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
					state = Utility::MakeDword(low, high);
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
		*pos = int(1 + current_ - start_ - bi);
	}

	return tokenType;
}

FileScanner::FileScanner(const char* fileName)
	: reader_(new FileReader(fileName, true)), lineno_(0) {
}

FileScanner::~FileScanner() {
	delete reader_;
}

bool FileScanner::GetToken(ScannerToken* token, TokenPosition* pos) {
	std::string line;
	std::string buffer;
	ScannerTokenType tokenType = textScanner_.GetToken(buffer, &pos->linepos);
	std::string code;
	for (; tokenType == ScannerTokenCode || tokenType == ScannerTokenEndOfFile; ) {
		if (tokenType == ScannerTokenCode) {
			textScanner_.Discard();
			return ReadCode(token, code);
		}

		if (!reader_->ReadLine(line, &lineno_)) {
			tokenType = ScannerTokenEndOfFile;
			break;
		}

		if (Utility::IsBlankText(line.c_str())) {
			continue;
		}

		textScanner_.SetText(line);
		tokenType = textScanner_.GetToken(buffer, &pos->linepos);
	}

	pos->lineno = lineno_;

	if(tokenType == ScannerTokenError) {
		return false;
	}

	token->tokenType = tokenType;
	token->tokenText = buffer;

	return true;
}

bool FileScanner::ReadCode(ScannerToken* token, std::string &code) {
	std::string line;
	for (; reader_->ReadLine(line, nullptr);) {
		std::string str = Utility::Trim(line);
		if (!str.empty() && str.front() == '@') {
			token->tokenType = ScannerTokenCode;
			token->tokenText = code;
			return true;
		}

		if (!code.empty()) {
			code += '\n';
		}

		code += line;
	}

	token->tokenType = ScannerTokenError;
	return false;
}
