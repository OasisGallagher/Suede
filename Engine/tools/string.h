#pragma once
#include <string>
#include <vector>

class String {
public:
	static std::string Trim(const std::string& text);
	static std::string Format(const char* format, ...);
	static void Split(const std::string& text, char sep, std::vector<std::string>& container);

	static bool StartsWith(const std::string& str, const std::string& prefix);
	static bool EndsWith(const std::string&str, const std::string& suffix);

	static int ToInteger(const std::string& str);
	static float ToFloat(const std::string& str);

	static std::wstring MultiBytesToWideString(const std::string& text);

private:
	String();
};

