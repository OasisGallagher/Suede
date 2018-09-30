#pragma once
#include <regex>
#include <vector>
#include "../types.h"

#include "../containers/sortedvector.h"

class FileTree;
class SUEDE_API FileEntry {
	friend class FileTree;

public:
	~FileEntry();

public:
	std::string GetPath() const { return path_; }
	void SetPath(const std::string& path, bool directory);
	bool IsDirectory() const { return directory_; }

	void AddChild(FileEntry* child) { children_.push_back(child); }
	uint GetChildCount() const { return (uint)children_.size(); }
	FileEntry* GetChildAt(uint i) { return children_[i]; }

private:
	bool directory_;
	std::string path_;
	std::vector<FileEntry*> children_;
};

struct FileEntryPtrComparer {
	bool operator ()(FileEntry* lhs, FileEntry* rhs) const {
		return lhs->GetPath() < rhs->GetPath();
	}
};

class SUEDE_API FileTree {
public:
	typedef sorted_vector<FileEntry*, FileEntryPtrComparer> EntryContainer;

public:
	FileTree();
	~FileTree();

public:
	bool Create(const std::string& directory, const std::string& reg);
	bool Reload(const std::string& path, const std::string& reg);
	FileEntry* GetRoot() { return root_; }
	const EntryContainer& GetAllEntries() const;

private:
	FileEntry* FindEntryAlongPath(const std::string& path);
	FileEntry* FindDirectChild(FileEntry* entry, const std::string& p);

	void BeforeRemoveChildEntiries(FileEntry* entry);
	bool CreateRecursively(FileEntry* parentNode, const std::string& path, const std::regex& r);

private:
	FileEntry* root_;
	EntryContainer entries_;
};

class SUEDE_API FileSystem {
public:
	static time_t GetFileLastWriteTime(const std::string& fileName, time_t defaultValue = 0);
	static bool ListFileTree(FileTree& tree, const std::string& directory, const std::string& reg = ".+");
	static bool ListAllFiles(std::vector<std::string>& paths, const std::string& directory, const std::string& reg = ".+");

	static std::string GetFileName(const std::string& path);
	static std::string GetFileNameWithoutExtension(const std::string& path);

	static std::string GetExtension(const std::string& path);
	static std::string GetParentPath(const std::string& path);
	//static std::string GetRelativePath(const std::string& directory, const std::string& path);

	static bool ReadAllText(const std::string& file, std::string& text);
	static bool ReadAllLines(const std::string& file, std::vector<std::string>& lines);

public:
	static bool ReadInteger(std::ifstream& file, int* x);
	static bool WriteInteger(std::ofstream& file, int x);

	static const uint kMaxStringLength = 256;
	static bool ReadString(std::ifstream& file, std::string* str);
	static bool WriteString(std::ofstream& file, const std::string& str);

private:
	FileSystem();
};
