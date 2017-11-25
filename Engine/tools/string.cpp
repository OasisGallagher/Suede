#include "debug.h"
#include "string.h"
#include <cstdarg>

std::string String::Format(const char* format, ...) {
	const int formatBufferLength = 512;
	static char formatBuffer[formatBufferLength];

	va_list ap;
	va_start(ap, format);
	int n = vsnprintf(formatBuffer, formatBufferLength, format, ap);
	va_end(ap);

	AssertX(n >= 0 && n < formatBufferLength, "format error");

	return formatBuffer;
}

void String::Split(const std::string& text, char sep, std::vector<std::string>& container) {
	if (text.empty()) { return; }

	size_t from = 0, pos;
	do {
		pos = text.find(sep, from);
		if (pos == std::string::npos) {
			container.push_back(text.substr(from, pos));
			break;
		}

		container.push_back(text.substr(from, pos - from));
		from = pos + 1;
	} while (true);
}

bool String::StartsWith(const std::string & str, const std::string & prefix) {
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool String::EndsWith(const std::string& str, const std::string& suffix) {
	return (str.length() >= suffix.length()) && (0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix));
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

std::string String::Trim(const std::string& text) {
	const char* whitespaces = " \t";
	size_t left = text.find_first_not_of(whitespaces);
	size_t right = text.find_last_not_of(whitespaces);
	if (left == std::string::npos || right == std::string::npos) {
		return text;
	}

	return std::string(text.begin() + left, text.begin() + right + 1);
}
