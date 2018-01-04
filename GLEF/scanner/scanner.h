#pragma once

#include <string>
#include "tokendefine.h"

class TextScanner {
public:
	TextScanner();
	~TextScanner();

	void SetText(const std::string& value);
	ScannerTokenType GetToken(std::string& token, int* pos = nullptr);
	void Discard() { current_ = nullptr; }

private:
	bool GetChar(int* ch);
	void UngetChar();
	ScannerTokenType GetNextToken(std::string& token, int* pos = nullptr);

private:
	const char* start_;
	const char* dest_;
	const char* current_;

	std::string lineBuffer_;
};

struct TokenPosition {
	int lineno;
	int linepos;

	std::string ToString() const;
};

class SourceScanner {
public:
	SourceScanner();
	~SourceScanner();

public:
	bool Open(const std::string& path);
	bool GetToken(ScannerToken* token, TokenPosition* pos);

private:
	bool ReadCode(ScannerToken* token);

private:
	int lineno_;
	std::string text_;
	const char* start_;
	TextScanner textScanner_;
};
