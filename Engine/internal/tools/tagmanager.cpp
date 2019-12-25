#include "tagmanager.h"
#include "debug/debug.h"

static std::vector<std::string> tags{ "DefaultTag0", "DefaultTag1", "DefaultTag2" };

const std::vector<std::string>& TagManager::GetAllRegisteredTags() {
	return tags;
}

void TagManager::Register(const std::string& name) {
	if (IsRegistered(name)) {
		Debug::LogError("duplicate tag %s.", name.c_str());
	}
	else {
		tags.push_back(name);
	}
}

void TagManager::Unregister(const std::string& name) {
	std::vector<std::string>::iterator pos = std::find(tags.begin(), tags.end(), name);
	if (pos != tags.end()) {
		tags.erase(pos);
	}
}

bool TagManager::IsRegistered(const std::string& name) {
	return std::find(tags.begin(), tags.end(), name) != tags.end();
}
