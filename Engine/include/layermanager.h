#pragma once
#include "types.h"

struct Layer {
	std::string name;
	uint value;
};

struct SUEDE_API LayerManager {
	static uint Default;
	static uint IgnorePick;

	static uint NameToLayer(const std::string& name);
	static std::string LayerToName(uint layer);

	static uint AddLayer(const std::string& name);
};
