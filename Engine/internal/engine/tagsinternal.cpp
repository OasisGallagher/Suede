#include "tagsinternal.h"

#include "debug/debug.h"

Tags::Tags() : Subsystem(new TagsInternal()) {}
void Tags::Register(const std::string& name) { _suede_dptr()->Register(name); }
void Tags::Unregister(const std::string& name) { _suede_dptr()->Unregister(name); }
bool Tags::IsRegistered(const std::string& name) { return _suede_dptr()->IsRegistered(name); }
const std::vector<std::string>& Tags::GetAllRegisteredTags() { return _suede_dptr()->GetAllRegisteredTags(); }

const std::vector<std::string>& TagsInternal::GetAllRegisteredTags() {
	return tags_;
}

void TagsInternal::Register(const std::string& name) {
	if (IsRegistered(name)) {
		Debug::LogError("duplicate tag %s.", name.c_str());
	}
	else {
		tags_.push_back(name);
	}
}

void TagsInternal::Unregister(const std::string& name) {
	std::vector<std::string>::iterator pos = std::find(tags_.begin(), tags_.end(), name);
	if (pos != tags_.end()) {
		tags_.erase(pos);
	}
}

bool TagsInternal::IsRegistered(const std::string& name) {
	return std::find(tags_.begin(), tags_.end(), name) != tags_.end();
}
