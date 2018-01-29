#include <fstream>
#include <filesystem>

#include "filesystem.h"
#include "../debug/debug.h"
#include "../tools/string.h"

namespace fs = std::experimental::filesystem::v1;

time_t FileSystem::GetFileLastWriteTime(const char* fileName) {
	std::error_code err;
	return fs::file_time_type::clock::to_time_t(fs::last_write_time(fileName, err));
}

void FileSystem::ListAllFiles(std::vector<std::string>& paths, const char* directory, const char* postfix) {
	std::string post;
	if (postfix != nullptr) { (post += ".") += postfix; }
	for (auto& p : fs::recursive_directory_iterator(directory)) {
		std::string str = p.path().string();
		if (fs::is_regular_file(p) && (postfix == nullptr || String::EndsWith(str, post))) {
			paths.push_back(str);
		}
	}
}

bool FileSystem::ReadAllText(const std::string& file, std::string& text) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs) {
		Debug::LogError("failed to open file %s.", file.c_str());
		return false;
	}

	text.clear();
	const char* seperator = "";

	std::string line;
	for (; getline(ifs, line);) {
		if (!line.empty()) {
			text += seperator;
			text += line;
			seperator = "\n";
		}
	}

	ifs.close();

	return true;
}

bool FileSystem::ReadAllLines(const std::string& file, std::vector<std::string>& lines) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs) {
		Debug::LogError("failed to open file %s.", file.c_str());
		return false;
	}

	lines.clear();

	std::string line;
	for (; getline(ifs, line);) {
		if (!line.empty()) {
			lines.push_back(line);
		}
	}

	ifs.close();

	return true;
}

std::string FileSystem::GetFileName(const std::string& path) {
	size_t slash = path.find_last_of("\\/");
	if (slash == std::string::npos) {
		return path;
	}

	return path.substr(slash + 1);
}

std::string FileSystem::GetFileNameWithoutExtension(const std::string& path) {
	std::string name = GetFileName(path);
	return name.substr(0, name.find_last_of('.'));
}

std::string FileSystem::GetDirectory(const std::string& path) {
	std::string dir;
	std::string::size_type slash = path.find_last_of("/");

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
