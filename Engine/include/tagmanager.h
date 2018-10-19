#pragma once
#include <vector>

#include "types.h"
#include "tools/singleton.h"

typedef std::vector<std::string> Tags;
class SUEDE_API TagManager : public Singleton2<TagManager> {
	friend class Singleton2<TagManager>;

public:
	const Tags& GetAllTags();

	void Register(const std::string& name);
	void Unregister(const std::string& name);
	bool IsRegistered(const std::string& name);

private:
	TagManager();
};
