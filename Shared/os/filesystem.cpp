#include <fstream>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#include "filesystem.h"
#include "../debug/debug.h"
#include "../tools/string.h"
#include "../memory/refptr.h"

namespace fs = std::experimental::filesystem;

static char intBuffer[sizeof(int)];
static char strBuffer[FileSystem::kMaxStringLength];

FileEntry::~FileEntry() {
	for (uint i = 0; i < children_.size(); ++i) {
		delete children_[i];
	}
}

void FileEntry::SetPath(const std::string& path, bool directory) {
	directory_ = directory;
	path_ = path;
}

FileTree::FileTree() : root_(nullptr) {
}

FileTree::~FileTree() {
	delete root_;
}

bool FileTree::Create(const std::string& directory, const std::string& reg) {
	delete root_;

	root_ = new FileEntry;
	root_->SetPath(directory, true);

	if (CreateRecursively(root_, root_->GetPath(), std::regex(reg))) {
		delete root_;
		root_ = nullptr;
		return false;
	}

	return true;
}

bool FileTree::Reload(const std::string& path, const std::string& reg) {
	FileEntry* entry = FindEntryAlongPath(path);
	if (entry == nullptr) {
		return false;
	}

	BeforeRemoveChildEntiries(entry);

	for (uint i = 0; i < entry->children_.size(); ++i) {
		delete entry->children_[i];
	}

	entry->children_.clear();

	CreateRecursively(entry, entry->GetPath(), std::regex(reg));
	return true;
}

void FileTree::BeforeRemoveChildEntiries(FileEntry* entry) {
	for (uint i = 0; i < entry->GetChildCount(); ++i) {
		FileEntry* child = entry->GetChildAt(i);
		entries_.erase(child);

		BeforeRemoveChildEntiries(child);
	}
}

const FileTree::EntryContainer& FileTree::GetAllEntries() const {
	return entries_;
}

FileEntry* FileTree::FindEntryAlongPath(const std::string& path) {
	std::vector<std::string> names;
	String::Split(names, path, '/');

	FileEntry* entry = nullptr;
	for (const std::string& p : names) {
		if (entry != nullptr) {
			entry = FindDirectChild(entry, p);
		}
		else if (p == FileSystem::GetFileName(root_->GetPath())) {
			entry = root_;
		}

		if (entry == nullptr) { break; }
	}

	return entry;
}

FileEntry* FileTree::FindDirectChild(FileEntry* entry, const std::string& p) {
	FileEntry* answer = nullptr;
	for (uint i = 0; i < entry->GetChildCount(); ++i) {
		FileEntry* child = entry->GetChildAt(i);
		if (FileSystem::GetFileName(child->GetPath()) == p) {
			answer = child;
			break;
		}
	}

	return answer;
}

bool FileTree::CreateRecursively(FileEntry* parentNode, const std::string& path, const std::regex& r) {
	bool empty = true;

	for (auto& p : fs::directory_iterator(path)) {
		FileEntry* entry;
		std::string name = p.path().filename().string();
		std::string childPath = path + "/" + name;
		if (fs::is_directory(p)) {
			entry = new FileEntry;
			entry->SetPath(childPath, true);

			if (!CreateRecursively(entry, childPath, r)) {
				entries_.insert(entry);
				parentNode->AddChild(entry);
			}
			else {
				delete entry;
			}
		}
		else if (fs::is_regular_file(p) && std::regex_match(name, r)) {
			empty = false;

			entry = new FileEntry;
			entry->SetPath(childPath, false);

			entries_.insert(entry);
			parentNode->AddChild(entry);
		}
	}

	return empty;
}

time_t FileSystem::GetFileLastWriteTime(const std::string& fileName, time_t defaultValue) {
	std::error_code err;
	fs::file_time_type time = fs::last_write_time(fileName, err);
	if (err) {
		return defaultValue;
	}

	return fs::file_time_type::clock::to_time_t(time);
}

bool FileSystem::ListFileTree(FileTree& tree, const std::string& directory, const std::string& reg) {
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

//std::string FileSystem::GetRelativePath(const std::string& directory, const std::string& path) {
//	fs::path::relative
//	//return fs::relative(childPath, parentPath);
//}

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

	ifs.close();

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

	ifs.close();
	return true;
}

bool FileSystem::WriteInteger(std::ofstream& file, int x) {
	return !!file.write((char*)&x, sizeof(x));
}

bool FileSystem::WriteString(std::ofstream& file, const std::string& str) {
	if (str.length() >= kMaxStringLength) {
		Debug::LogError("string length exceed.");
		return false;
	}

	int count = (int)str.length();
	return file.write((char*)&count, sizeof(count)) && file.write(str.c_str(), count);
}

bool FileSystem::ReadInteger(std::ifstream& file, int* x) {
	if (!file.read(intBuffer, sizeof(int))) {
		return false;
	}

	if (x != nullptr) {
		*x = *(int*)intBuffer;
	}

	return true;
}

bool FileSystem::ReadString(std::ifstream& file, std::string* str) {
	int length = 0;
	if (!ReadInteger(file, &length)) {
		return false;
	}

	if (length >= kMaxStringLength) {
		Debug::LogError("token length exceed.");
		return false;
	}

	if (!file.read(strBuffer, length)) {
		return false;
	}

	if (str != nullptr) {
		strBuffer[length] = 0;
		str->assign(strBuffer);
	}

	return true;
}
