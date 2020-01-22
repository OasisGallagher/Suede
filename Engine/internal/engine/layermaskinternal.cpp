#include "layermaskinternal.h"

#include "debug//debug.h"

LayerMask::LayerMask() : Subsystem(new LayerMaskInternal()) {}
int LayerMask::GetMask(std::initializer_list<std::string> names) { return _suede_dptr()->GetMask(names); }
int LayerMask::GetAllLayers() { return _suede_dptr()->GetAllLayers(); }
std::string LayerMask::LayerToName(int layerIndex) { return _suede_dptr()->LayerToName(layerIndex); }
int LayerMask::NameToLayer(const std::string& name) { return _suede_dptr()->NameToLayer(name); }

#define MAKE_LAYER(name, value)	std::make_pair(std::string(name), 1 << value)
static const auto defaultLayers = {
	MAKE_LAYER("Default", 0),
	MAKE_LAYER("Selected", 1)
};

LayerMaskInternal::LayerMaskInternal() : layerMasks_(defaultLayers) {
}

int LayerMaskInternal::GetMask(std::initializer_list<std::string> names) {
	int mask = 0;
	for (const std::string& name : names) {
		int index = NameToLayer(name);
		if (index == -1) {
			Debug::LogWarning("Invalid layer name %s", name.c_str());
			continue;
		}

		mask |= layerMasks_[index].second;
	}

	return mask;
}

int LayerMaskInternal::GetAllLayers() {
	int answer = 0;
	for (int i = 0; i < layerMasks_.size(); ++i) {
		answer |= (1 << i);
	}

	return answer;
}

std::string LayerMaskInternal::LayerToName(int layerIndex) {
	if (layerIndex >= layerMasks_.size()) {
		Debug::LogError("Invalid layer index %d", layerIndex);
		return "";
	}

	return layerMasks_[layerIndex].first;
}

int LayerMaskInternal::NameToLayer(const std::string& name) {
	for (int i = 0; i < layerMasks_.size(); ++i) {
		if (layerMasks_[i].first == name) {
			return i;
		}
	}

	return -1;
}
