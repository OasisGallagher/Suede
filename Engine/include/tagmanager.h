#pragma once
#include <vector>

#include "types.h"
#include "tools/singleton.h"

class SUEDE_API TagManager : public Singleton2<TagManager> {
public:
	virtual void Register(const std::string& name) = 0;
	virtual void Unregister(const std::string& name) = 0;
	virtual bool IsRegistered(const std::string& name) = 0;
	virtual void GetAllTags(std::vector<std::string>& container) = 0;
};
