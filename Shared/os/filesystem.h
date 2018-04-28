#pragma once
#include <regex>
#include <vector>
#include "../shareddefines.h"

class SHARED_API FileEntry {
public:
	~FileEntry();

public:
	const std::string& GetName() const { return name_; }
	void SetName(const std::string& value) { name_ = value; }
	bool IsDirectory() const { return name_.back() == '/'; }

	void AddChild(FileEntry* child) { children_.push_back(child); }
	uint GetChildCount() const { return children_.size(); }
	FileEntry* GetChildAt(uint i) { return children_[i]; }

private:
	std::string name_;
	std::vector<FileEntry*> children_;
};

class SHARED_API FileTree {
public:
	FileTree();
	~FileTree();

public:
	bool Create(const std::string& directory, const std::string& reg);
	FileEntry* GetRoot() { return root_; }

private:
	std::string EnsureDirectory(const std::string& directory);
	bool CreateRecursively(FileEntry* parentNode, const std::string& parentDirectory, const std::string& name, const std::regex& r);

private:
	FileEntry* root_;
};

class SHARED_API FileSystem {
public:
	static time_t GetFileLastWriteTime(const std::string& fileName);
	static bool ListAllFiles(FileTree& tree, const std::string& directory, const std::string& reg = ".+");
	static bool ListAllFiles(std::vector<std::string>& paths, const std::string& directory, const std::string& reg = ".+");

	static std::string GetFileName(const std::string& path);
	static std::string GetFileNameWithoutExtension(const std::string& path);

	static std::string GetExtension(const std::string& path);
	static std::string GetParentPath(const std::string& path);

	static bool ReadAllText(const std::string& file, std::string& text);
	static bool ReadAllLines(const std::string& file, std::vector<std::string>& lines);

private:
	FileSystem();
};
