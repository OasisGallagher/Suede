#include <regex>
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

void FileSystem::ListAllFiles(std::vector<std::string>& paths, const char* directory, const char* reg) {
	std::regex r(reg);
	for (auto& p : fs::recursive_directory_iterator(directory)) {
		std::string str = p.path().string();
		if (fs::is_regular_file(p) && std::regex_match(str, r)) {
			paths.push_back(str);
		}
	}
}

std::string FileSystem::GetFileName(const std::string& path) {
	return fs::path(path).filename().string();
}

std::string FileSystem::GetFileNameWithoutExtension(const std::string& path) {
	return fs::path(path).stem().string();
}

std::string FileSystem::GetParentPath(const std::string& path) {
	return fs::path(path).parent_path().string();
}

bool FileSystem::ReadAllText(const std::string& file, std::string& text) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs) {
		Debug::LogError("failed to open file %s.", file.c_str());
		return false;
	}

	const char* seperator = "";
	for (std::string line; std::getline(ifs, line); seperator = "\n") {
		text += (seperator + line);
	}

	return true;
}

bool FileSystem::ReadAllLines(const std::string& file, std::vector<std::string>& lines) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs) {
		Debug::LogError("failed to open file %s.", file.c_str());
		return false;
	}

	for (std::string line; std::getline(ifs, line);) {
		lines.push_back(line);
	}

	return true;
}
