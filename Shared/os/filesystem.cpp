#include <fstream>
#include <filesystem>

#include "filesystem.h"
#include "../debug/debug.h"
#include "../tools/string.h"
#include "../memory/memory.h"

namespace fs = std::experimental::filesystem::v1;

FileTree::FileTree() : root_(nullptr) {
}

FileTree::~FileTree() {
	MEMORY_RELEASE(root_);
}

FileEntry::~FileEntry() {
	for (uint i = 0; i < children_.size(); ++i) {
		MEMORY_RELEASE(children_[i]);
	}
}

bool FileTree::Create(const std::string& directory, const std::string& reg) {
	MEMORY_RELEASE(root_);

	root_ = MEMORY_CREATE(FileEntry);
	root_->SetName(EnsureDirectory(directory));

	if (CreateRecursively(root_, root_->GetName(), "", std::regex(reg))) {
		MEMORY_RELEASE(root_);
		root_ = nullptr;
		return false;
	}

	return true;
}

std::string FileTree::EnsureDirectory(const std::string& directory) {
	if (directory.back() == '/') { return directory; };
	return directory + '/';
}

bool FileTree::CreateRecursively(FileEntry* parentNode, const std::string& parentDirectory, const std::string& directory, const std::regex& r) {
	bool empty = true;
	std::string path = parentDirectory + directory;

	for (auto& p : fs::directory_iterator(path)) {
		std::string name = p.path().filename().string();
		FileEntry* entry;
		if (fs::is_directory(p)) {
			entry = MEMORY_CREATE(FileEntry);
			name += "/";
			entry->SetName(name);

			if (!CreateRecursively(entry, path, name, r)) {
				parentNode->AddChild(entry);
			}
			else {
				MEMORY_RELEASE(entry);
			}
		}
		else if (fs::is_regular_file(p) && std::regex_match(name, r)) {
			empty = false;
			entry = MEMORY_CREATE(FileEntry);
			entry->SetName(path + name);
			parentNode->AddChild(entry);
		}
	}

	return empty;
}

time_t FileSystem::GetFileLastWriteTime(const std::string& fileName) {
	std::error_code err;
	return fs::file_time_type::clock::to_time_t(fs::last_write_time(fileName, err));
}

bool FileSystem::ListAllFiles(FileTree& tree, const std::string& directory, const std::string& reg) {
	return tree.Create(directory, reg);
}

bool FileSystem::ListAllFiles(std::vector<std::string>& paths, const std::string& directory, const std::string& reg) {
	bool empty;
	std::regex r(reg);
	for (auto& p : fs::recursive_directory_iterator(directory)) {
		std::string str = p.path().string();
		if (fs::is_regular_file(p) && std::regex_match(str, r)) {
			paths.push_back(str);
			empty = false;
		}
	}

	return !empty;
}

std::string FileSystem::GetFileName(const std::string& path) {
	return fs::path(path).filename().string();
}

std::string FileSystem::GetFileNameWithoutExtension(const std::string& path) {
	return fs::path(path).stem().string();
}

std::string FileSystem::GetExtension(const std::string& path) {
	return fs::path(path).extension().string();
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
