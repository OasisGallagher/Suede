#pragma once
#include <vector>

#include "types.h"
#include "tools/singleton.h"

typedef std::vector<std::string> Tags;
class SUEDE_API TagManager : public Singleton2<TagManager> {
public:
	virtual const Tags& GetAllTags() = 0;

	virtual void Register(const std::string& name) = 0;
	virtual void Unregister(const std::string& name) = 0;
	virtual bool IsRegistered(const std::string& name) = 0;
};
