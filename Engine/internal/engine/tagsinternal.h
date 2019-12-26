#pragma once
#include "tags.h"
#include "subsysteminternal.h"

class TagsInternal : public SubsystemInternal {
public:
	void Register(const std::string& name);
	void Unregister(const std::string& name);
	bool IsRegistered(const std::string& name);
	const std::vector<std::string>& GetAllRegisteredTags();

private:
	std::vector<std::string> tags_{ 
		"DefaultTag0", 
		"DefaultTag1", 
		"DefaultTag2" 
	};
};
