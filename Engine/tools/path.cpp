#include "path.h"

std::string Path::GetDirectory(const std::string& path) {
	std::string dir;
	unsigned slash = (unsigned)path.find_last_of("/");

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
