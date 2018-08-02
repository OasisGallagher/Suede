#pragma once
#include "tagmanager.h"

#include <set>

class TagManagerInternal : public TagManager {
public:
	TagManagerInternal();

public:
	virtual void Register(const std::string& name);
	virtual void Unregister(const std::string& name);
	virtual bool IsRegistered(const std::string& name);
	virtual void GetAllTags(std::vector<std::string>& container);

private:
	typedef std::set<std::string> TagContainer;
	TagContainer tagContainer;
};