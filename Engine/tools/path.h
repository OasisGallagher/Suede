#pragma once
#include <string>

class Path {
public:
	static std::string GetFileName(const std::string& path);
	static std::string GetDirectory(const std::string& path);
	static std::string GetResourceRootDirectory();
};

inline std::string Path::GetResourceRootDirectory() {
	return "resources/";
}
