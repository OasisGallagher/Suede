#pragma once
#include <vector>

#include "layermask.h"
#include "subsysteminternal.h"

class LayerMaskInternal : public SubsystemInternal {
public:
	LayerMaskInternal();

public:
	int GetMask(std::initializer_list<std::string> names);
	int GetAllLayers();
	std::string LayerToName(int layerIndex);
	int NameToLayer(const std::string& name);

private:
	std::vector<std::pair<std::string, int>> layerMasks_;
};
