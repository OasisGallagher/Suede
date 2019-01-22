#include "string.h"
#include <cstdarg>

#define HEADING_LENGTH			48

// https://github.com/FooBarWidget/boyer-moore-horspool
// https://softwareengineering.stackexchange.com/questions/183725/which-string-search-algorithm-is-actually-the-fastest
// 
// Boyer-Moore: 
//   works by pre-analyzing the pattern and comparing from right-to-left. 
//   If a mismatch occurs, the initial analysis is used to determine how far
//   the pattern can be shifted w.r.t. the text being searched. This works 
//   particularly well for long search patterns. In particular, it can be 
//   sub-linear, as you do not need to read every single character of your text.
// Knuth-Morris-Pratt: 
//   also pre-analyzes the pattern, but tries to re-use whatever was already
//   matched in the initial part of the pattern to avoid having to rematch that. 
//   This can work quite well, if your alphabet is small (f.ex. DNA bases), as 
//   you get a higher chance that your search patterns contain reuseable subpatterns.
BoyerMoor::BoyerMoor(const char* needle, size_t length) {
	// initialize a table of UCHAR_MAX+1 elements to value needle_length
	occ_.assign(UCHAR_MAX + 1, length);
	needle_.assign(needle, needle + length);

	// Populate it with the analysis of the needle.
	// But ignoring the last letter.
	if (length >= 1) {
		const size_t needle_length_minus_1 = length - 1;
		for (size_t i = 0; i < needle_length_minus_1; ++i) {
			occ_[(unsigned char)needle[i]] = needle_length_minus_1 - i;
		}
	}
}

size_t BoyerMoor::Search(const char* haystack, size_t length) {
	if (needle_.length() > length) { return length; }
	if (needle_.length() == 1) {
		const char* result = (const char*)memchr(haystack, *needle_.c_str(), length);
		return result ? size_t(result - haystack) : length;
	}

	const size_t needle_length_minus_1 = needle_.length() - 1;
	const char last_needle_char = needle_[needle_length_minus_1];

	size_t haystack_position = 0;
	while (haystack_position <= length - needle_.length()) {
		const char occ_char = haystack[haystack_position + needle_length_minus_1];

		// The author modified this part. Original algorithm matches needle right-to-left.
		// This code calls memcmp() (usually matches left-to-right) after matching the last
		// character, thereby incorporating some ideas from
		// "Tuning the Boyer-Moore-Horspool String Searching Algorithm"
		// by Timo Raita, 1992.
		if (last_needle_char == occ_char
			&& _strnicmp(needle_.c_str(), haystack + haystack_position, needle_length_minus_1) == 0) {
			return haystack_position;
		}

		haystack_position += occ_[(unsigned char)occ_char];
	}

	return length;
}

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

bool String::ToBool(const std::string& str, bool* value) {
	if (str == "true") {
		if (value != nullptr) *value = true;
		return true;
	}

	if (str == "false") {
		if (value != nullptr) *value = false;
		return true;
	}

	return false;
}

int String::ToInteger(const std::string& str) {
	int ans = 0;
	ToInteger(str, &ans);
	return ans;
}

bool String::ToInteger(const std::string& str, int* value) {
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
	if (value != nullptr) {
		*value = ans;
	}

	return (*p == 0);
}

bool String::ToFloat(const std::string& str, float* value) {
	if (str.empty() || ((!IsDigit(str[0])) && (str[0] != '-') && (str[0] != '+'))) {
		return false;
	}

	char* p = nullptr;
	float ans = strtof(str.c_str(), &p);
	if (value != nullptr) {
		*value = ans;
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
