#pragma once
#include <vector>
#include "../shareddefines.h"

class SHARED_API File {
public:
	static bool Load(const std::string& file, std::string& text);
	static bool Load(const std::string& file, std::vector<std::string>& lines);
};
