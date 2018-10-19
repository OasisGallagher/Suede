#pragma once
#include "tagmanager.h"

class TagManagerInternal {
public:
	TagManagerInternal();

public:
	virtual const Tags& GetAllTags() { return tags; }

	virtual void Register(const std::string& name);
	virtual void Unregister(const std::string& name);
	virtual bool IsRegistered(const std::string& name);

private:
	Tags tags;
};