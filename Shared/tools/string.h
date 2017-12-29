#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "../shareddefines.h"

class SHARED_API String {
public:
	static std::string Trim(const std::string& text);
	static const char* TrimStart(const char* text);
	static std::string Format(const char* format, ...);

	static bool StartsWith(const std::string& str, const std::string& prefix);
	static bool EndsWith(const std::string&str, const std::string& suffix);

	static int ToInteger(const std::string& str);
	static bool ToInteger(const std::string& str, int* integer);
	static float ToFloat(const std::string& str);

	static bool SplitLine(const char*& ptr, std::string& line);

	static bool IsBlankText(const char* text, const char** pos = nullptr);

	static bool IsDigit(int c);
	static bool IsLetter(int c);

	template <class Iterator>
	static std::string Concat(Iterator first, Iterator last, const char* seperator = " ");
	static std::string Heading(const std::string& text);

	static void Split(std::vector<std::string>& answer, const std::string& str, char seperator);
	static std::wstring MultiBytesToWideString(const std::string& text);

private:
	String();
};

inline bool String::IsDigit(int c) {
	return isdigit(c) != 0;
}

inline bool String::IsLetter(int c) {
	return (isalpha(c) != 0) || c == '_' || c == '$';
}

template <class Iterator>
std::string String::Concat(Iterator first, Iterator last, const char* seperator) {
	const char* sep = "";
	std::ostringstream oss;

	for (; first != last; ++first) {
		oss << sep;
		oss << first->ToString();
		sep = seperator;
	}

	return oss.str();
}
