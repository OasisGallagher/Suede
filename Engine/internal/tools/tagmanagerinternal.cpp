#include "tagmanagerinternal.h"

#include <set>
#include "debug/debug.h"
#include "tools/math2.h"

static const char* defaultTags[] = { "DefaultTag0", "DefaultTag1", "DefaultTag2" };

TagManagerInternal::TagManagerInternal() : tagContainer(defaultTags, defaultTags + CountOf(defaultTags)) {
}

void TagManagerInternal::Register(const std::string& name) {
	if (!tagContainer.insert(name).second) {
		Debug::LogError("duplicate tag %s.", name.c_str());
	}
}

void TagManagerInternal::Unregister(const std::string& name) {
	tagContainer.erase(name);
}

bool TagManagerInternal::IsRegistered(const std::string& name) {
	return tagContainer.count(name) != 0;
}

void TagManagerInternal::GetAllTags(std::vector<std::string>& container) {
	container.assign(tagContainer.begin(), tagContainer.end());
}