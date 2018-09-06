#include "string.h"
#include <cstdarg>

#define HEADING_LENGTH			48

std::string String::Format(const char* format, ...) {
	va_list args;
	va_start(args, format);
	std::string str = VFormat(format, args);
	va_end(args);
	return str;
}

std::string String::VFormat(const char* format, va_list args) {
	const int formatBufferLength = 512;
	char formatBuffer[formatBufferLength];

	int n = vsnprintf(formatBuffer, formatBufferLength, format, args);

	if (n < 0) {
		*formatBuffer = 0;
	}

	return formatBuffer;
}

bool String::StartsWith(const std::string & str, const std::string & prefix) {
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool String::EndsWith(const std::string& str, const std::string& suffix) {
	return (str.length() >= suffix.length()) && (0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix));
}

bool String::ToBool(const std::string& str) {
	bool ans = false;
	ToBool(str, &ans);
	return ans;
}

bool String::ToBool(const std::string& str, bool* boolean) {
	if (str == "true") {
		if (boolean != nullptr) *boolean = true;
		return true;
	}

	if (str == "false") {
		if (boolean != nullptr) *boolean = false;
		return true;
	}

	return false;
}

int String::ToInteger(const std::string& str) {
	int ans = 0;
	ToInteger(str, &ans);
	return ans;
}

bool String::ToInteger(const std::string& str, int* integer) {
	if (str.empty() || ((!IsDigit(str[0])) && (str[0] != '-') && (str[0] != '+'))) {
		return false;
	}

	char* p = nullptr;
	int base = 10;
	const char* ptr = str.c_str();
	
	if (_strnicmp(ptr, "0x", 2) == 0) {
		ptr += 2;
		base = 16;
	}
	
	int ans = strtol(ptr, &p, base);
	if (integer != nullptr) {
		*integer = ans;
	}

	return (*p == 0);
}

bool String::ToFloat(const std::string& str, float* single) {
	if (str.empty() || ((!IsDigit(str[0])) && (str[0] != '-') && (str[0] != '+'))) {
		return false;
	}

	char* p = nullptr;
	float ans = strtof(str.c_str(), &p);
	if (single != nullptr) {
		*single = ans;
	}

	return (*p == 0 || *p == 'f' || *p == 'F');
}

float String::ToFloat(const std::string& str) {
	return std::stof(str);
}

std::string String::Heading(const std::string& text) {
	if (HEADING_LENGTH < (int)text.length()) {
		return text;
	}

	size_t left = (HEADING_LENGTH - text.length()) / 2;
	std::string ans(HEADING_LENGTH, '=');
	for (size_t i = 0; i < text.length(); ++i) {
		ans[left++] = text[i];
	}

	return ans;
}

bool String::IsBlankText(const char* text, const char** pos) {
	const char* start = TrimStart(text);
	if (*start == 0) {
		return true;
	}

	if (pos != nullptr) {
		*pos = start;
	}

	return false;
}

std::string String::Trim(const std::string& text) {
	const char* whitespaces = " \t";
	size_t left = text.find_first_not_of(whitespaces);
	size_t right = text.find_last_not_of(whitespaces);
	if (left == std::string::npos || right == std::string::npos) {
		return text;
	}

	return std::string(text.begin() + left, text.begin() + right + 1);
}

void String::Split(std::vector<std::string>& answer, const std::string& str, char seperator) {
	if (str.empty()) { return; }

	size_t from = 0, pos;
	do {
		pos = str.find(seperator, from);
		if (pos == std::string::npos) {
			answer.push_back(str.substr(from, pos));
			break;
		}

		answer.push_back(str.substr(from, pos - from));
		from = pos + 1;
	} while (true);
}

std::wstring String::MultiBytesToWideString(const std::string& text) {
	std::wstring ans(text.size() + 1, 0);
	mbstowcs(&ans[0], text.c_str(), text.length());
	size_t pos = ans.find((wchar_t)0);
	
	if (pos < ans.length()) {
		ans.erase(pos);
	}

	return ans;
}

const char* String::TrimStart(const char* text) {
	return text + strspn(text, "\t ");
}
