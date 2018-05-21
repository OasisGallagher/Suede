#include <set>

#include "tagmanager.h"
#include "debug/debug.h"
#include "tools/math2.h"

typedef std::set<std::string> TagContainer;
static const char* defaultTags[] = { "" };
static TagContainer tagContainer(defaultTags, defaultTags + CountOf(defaultTags));

void TagManager::Register(const std::string& name) {
	if (!tagContainer.insert(name).second) {
		Debug::LogError("duplicate tag %s.", name.c_str());
	}
}

void TagManager::Unregister(const std::string& name) {
	tagContainer.erase(name);
}

bool TagManager::IsRegistered(const std::string& name) {
	return tagContainer.count(name) != 0;
}

void TagManager::GetAllTags(std::vector<std::string>& container) {
	container.assign(tagContainer.begin(), tagContainer.end());
}
