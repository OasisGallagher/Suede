#include "tagmanager.h"
#include "debug/debug.h"
#include "containers/sortedvector.h"

typedef SortedVector<std::string> TagContainer;
static TagContainer tagContainer;

void TagManager::Register(const std::string& name) {
	tagContainer.insert(name);
}

void TagManager::Unregister(const std::string& name) {
	tagContainer.remove(name);
}

bool TagManager::IsRegistered(const std::string& name) {
	return tagContainer.contains(name);
}

void TagManager::GetAllTags(std::vector<std::string>& container) {
	container.assign(tagContainer.begin(), tagContainer.end());
}
