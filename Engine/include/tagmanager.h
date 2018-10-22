#pragma once
#include <vector>

#include "types.h"
#include "enginedefines.h"
#include "tools/singleton.h"

typedef std::vector<std::string> Tags;
class SUEDE_API TagManager : public Singleton2<TagManager> {
	friend class Singleton<TagManager>;
	SUEDE_DECLARE_IMPLEMENTATION(TagManager)

public:
	const Tags& GetAllTags();

	void Register(const std::string& name);
	void Unregister(const std::string& name);
	bool IsRegistered(const std::string& name);

private:
	TagManager();
};
