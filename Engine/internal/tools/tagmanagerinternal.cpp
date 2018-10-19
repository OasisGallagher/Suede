#include "tagmanagerinternal.h"

#include <set>
#include "debug/debug.h"
#include "tools/math2.h"
#include "memory/memory.h"

#undef _dptr
#define _dptr()	((TagManagerInternal*)d_)
TagManager::TagManager() : Singleton2<TagManager>(MEMORY_NEW(TagManagerInternal)) {}
const Tags& TagManager::GetAllTags() { return _dptr()->GetAllTags(); }
void TagManager::Register(const std::string& name) { _dptr()->Register(name); }
void TagManager::Unregister(const std::string& name) { _dptr()->Unregister(name); }
bool TagManager::IsRegistered(const std::string& name) { return _dptr()->IsRegistered(name); }

static const char* defaultTags[] = { "DefaultTag0", "DefaultTag1", "DefaultTag2" };

TagManagerInternal::TagManagerInternal() : tags(defaultTags, defaultTags + SUEDE_COUNTOF(defaultTags)) {
}

void TagManagerInternal::Register(const std::string& name) {
	if (IsRegistered(name)) {
		Debug::LogError("duplicate tag %s.", name.c_str());
	}
	else {
		tags.push_back(name);
	}
}

void TagManagerInternal::Unregister(const std::string& name) {
	Tags::iterator pos = std::find(tags.begin(), tags.end(), name);
	if (pos != tags.end()) {
		tags.erase(pos);
	}
}

bool TagManagerInternal::IsRegistered(const std::string& name) {
	return std::find(tags.begin(), tags.end(), name) != tags.end();
}
