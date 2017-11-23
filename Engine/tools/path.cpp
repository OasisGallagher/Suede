#include "path.h"
#include "defines.h"

std::string Path::GetFileName(const std::string& path) {
	size_t slash = path.find_last_of("\\/");
	if (slash == std::string::npos) {
		return path;
	}

	return path.substr(slash + 1);
}

std::string Path::GetDirectory(const std::string& path) {
	std::string dir;
	uint slash = (uint)path.find_last_of("/");

	if (slash == std::string::npos) {
		dir = ".";
	}
	else if (slash == 0) {
		dir = "/";
	}
	else {
		dir = path.substr(0, slash);
	}

	return dir;
}
