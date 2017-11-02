#pragma once
#include <vector>

class TextLoader {
public:
	static bool Load(const std::string& file, std::string& text);
	static bool Load(const std::string& file, std::vector<std::string>& lines);
};
