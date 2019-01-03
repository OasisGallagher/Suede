#pragma once
#include <vector>

#include "types.h"
#include "enginedefines.h"
#include "tools/singleton.h"

typedef std::vector<std::string> Tags;
class SUEDE_API TagManager : private singleton2<TagManager> {
	friend class singleton<TagManager>;
	SUEDE_DECLARE_IMPLEMENTATION(TagManager)

public:
	static const Tags& GetAllTags();

	static void Register(const std::string& name);
	static void Unregister(const std::string& name);
	static bool IsRegistered(const std::string& name);

private:
	TagManager();
};
