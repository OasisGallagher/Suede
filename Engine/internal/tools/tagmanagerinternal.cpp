#include "tagmanagerinternal.h"

#include <set>
#include "debug/debug.h"
#include "tools/math2.h"

static const char* defaultTags[] = { "DefaultTag0", "DefaultTag1", "DefaultTag2" };

TagManagerInternal::TagManagerInternal() : tags(defaultTags, defaultTags + CountOf(defaultTags)) {
}

void TagManagerInternal::Register(const std::string& name) {
	if (!IsRegistered(name)) {
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
