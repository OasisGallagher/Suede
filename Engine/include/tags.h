#pragma once
#include <vector>
#include "types.h"
#include "subsystem.h"

class SUEDE_API Tags : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Tags)
public:
	enum {
		SystemType = SubsystemType::Tags,
	};

public:
	Tags();

public:
	void Register(const std::string& name);
	void Unregister(const std::string& name);
	bool IsRegistered(const std::string& name);
	const std::vector<std::string>& GetAllRegisteredTags();
};
