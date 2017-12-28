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

class FileReader;

struct TokenPosition {
	int lineno;
	int linepos;

	std::string ToString() const;
};

class FileScanner {
public:
	FileScanner(const char* path);
	~FileScanner();

public:
	bool GetToken(ScannerToken* token, TokenPosition* pos);

private:
	bool ReadCode(ScannerToken* token, std::string& code);

private:
	int lineno_;
	FileReader* reader_;
	TextScanner textScanner_;
};
