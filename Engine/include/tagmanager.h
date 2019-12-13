#pragma once
#include <vector>
#include "types.h"

class SUEDE_API TagManager {
public:
	static void Register(const std::string& name);
	static void Unregister(const std::string& name);
	static bool IsRegistered(const std::string& name);
	static const std::vector<std::string>& GetAllTags();
};
