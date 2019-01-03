#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <functional>

#include "../types.h"

class SUEDE_API BoyerMoor {
public:
	BoyerMoor(const char* needle, size_t length);

public:
	size_t Search(const char* haystack, size_t length);

private:
	std::string needle_;
	std::vector<size_t> occ_;
};

class SUEDE_API String {
public:
	static std::string Trim(const std::string& text);
	static const char* TrimStart(const char* text);
	static std::string Format(const char* format, ...);
	static std::string VFormat(const char* format, va_list args);

	static bool StartsWith(const std::string& str, const std::string& prefix);
	static bool EndsWith(const std::string&str, const std::string& suffix);

	static bool ToBool(const std::string& str);
	static bool ToBool(const std::string& str, bool* value);

	static int ToInteger(const std::string& str);
	static bool ToInteger(const std::string& str, int* value);
	static float ToFloat(const std::string& str);
	static bool ToFloat(const std::string& str, float* value);

	static bool IsBlankText(const char* text, const char** pos = nullptr);

	static bool IsDigit(int c);
	static bool IsLetter(int c);

	template <class Iterator>
	static std::string Concat(Iterator first, Iterator last, const char* seperator = " ");

	template <class Iterator, class ToStringF>
	static std::string Concat(Iterator first, Iterator last, const ToStringF& tostring, const char* seperator = " ");

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
	return Concat(first, last, [](typename std::iterator_traits<Iterator>::reference x) { return x.ToString(); });
}

template <class Iterator, class ToStringF>
std::string String::Concat(Iterator first, Iterator last, const ToStringF& tostring, const char* seperator) {
	const char* sep = "";
	std::ostringstream oss;

	for (; first != last; ++first) {
		oss << sep;
		oss << tostring(*first);
		sep = seperator;
	}

	return oss.str();
}

#include "../3rdparty/glm-0.9.7.1/include/glm/glm.hpp"

namespace std {
	// for completeness.
	inline std::string to_string(const std::string& value) { return value; }
	inline std::string to_string(const glm::vec2& value) { return String::Format("(%.2f, %.2f)", value.x, value.y); }
	inline std::string to_string(const glm::vec3& value) { return String::Format("(%.2f, %.2f, %.2f)", value.x, value.y, value.z); }
	inline std::string to_string(const glm::vec4& value) { return String::Format("(%.2f, %.2f, %.2f, %.2f)", value.x, value.y, value.z, value.w); }
}	// namespace std
