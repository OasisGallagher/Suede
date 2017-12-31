#include "string.h"
#include <cstdarg>

#define HEADING_LENGTH			48

std::string String::Format(const char* format, ...) {
	const int formatBufferLength = 512;
	static char formatBuffer[formatBufferLength];

	va_list ap;
	va_start(ap, format);
	int n = vsnprintf(formatBuffer, formatBufferLength, format, ap);
	va_end(ap);

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

int String::ToInteger(const std::string& str) {
	return std::stoi(str);
}

bool String::ToInteger(const std::string& str, int* integer) {
	if (str.empty() || ((!IsDigit(str[0])) && (str[0] != '-') && (str[0] != '+'))) {
		return false;
	}

	char* p = nullptr;
	int ans = strtol(str.c_str(), &p, 10);
	if (integer != nullptr) {
		*integer = ans;
	}

	return (*p == 0);
}

float String::ToFloat(const std::string& str) {
	return std::stof(str);
}

bool String::SplitLine(const char*& ptr, std::string& line) {
	if (*ptr == 0) { return false; }
	const char* start = ptr;
	for (; *ptr != 0; ++ptr) {
		if (*ptr == '\n') {
			line.assign(start, ptr++);
			break;
		}
	}

	if (*ptr == 0) {
		line.assign(start, ptr);
	}

	return true;
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
