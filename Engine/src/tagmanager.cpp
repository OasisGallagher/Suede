#include "tagmanager.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "containers/sortedvector.h"

typedef SortedVector<std::string> TagContainer;
static const char* defaultTags[] = { "" };
static TagContainer tagContainer(defaultTags, defaultTags + CountOf(defaultTags));

void TagManager::Register(const std::string& name) {
	tagContainer.insert(name);
}

void TagManager::Unregister(const std::string& name) {
	tagContainer.remove(name);
}

bool TagManager::IsRegistered(const std::string& name) {
	return tagContainer.contains(name);
}

const std::vector<std::string>& TagManager::GetAllTags() {
	return tagContainer.container();
}
