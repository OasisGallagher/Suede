#pragma once
#include <string>
#include "../shareddefines.h"

class SHARED_API Path {
public:
	static std::string GetFileName(const std::string& path);
	static std::string GetDirectory(const std::string& path);
	static std::string GetResourceRootDirectory();
};

inline std::string Path::GetResourceRootDirectory() {
	return "resources/";
}
