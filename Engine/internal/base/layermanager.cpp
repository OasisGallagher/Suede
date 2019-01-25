#include "layermanager.h"

#include "debug/debug.h"
#include "containers/sortedvector.h"

#define BUILTIN_LAYER_PREFIX			"_"
#define BUILTIN_LAYER_NAME(name)		(BUILTIN_LAYER_PREFIX #name)
#define DEFINE_BUILTIN_LAYER(name)		uint LayerManager::name = LayerManager::AddLayer((BUILTIN_LAYER_NAME(name)))

struct LayerComparer {
	bool operator()(const Layer& lhs, const Layer& rhs) const {
		return lhs.name < rhs.name;
	}
};

static sorted_vector<Layer, LayerComparer> s_layerMasks;

DEFINE_BUILTIN_LAYER(Default);
DEFINE_BUILTIN_LAYER(IgnoreRaycast);

uint LayerManager::NameToLayer(const std::string& name) {
	Layer layerMask = { name };
	if (!s_layerMasks.get(layerMask)) {
		Debug::LogError("can not find layermask named %s", name.c_str());
	}

	return layerMask.value;
}

std::string LayerManager::LayerToName(uint layer) {
	for (const Layer& mask : s_layerMasks) {
		if (mask.value == layer) {
			return mask.name;
		}
	}

	Debug::LogError("can not find layermask with layer %d", layer);
	return "";
}

uint LayerManager::AddLayer(const std::string& name) {
	Layer layerMask = { name, 1 << s_layerMasks.size() };
	if (s_layerMasks.contains(layerMask)) {
		Debug::LogError("layermask \"%s\" already exist", name);
		return 0;
	}

	s_layerMasks.insert(layerMask);
	return layerMask.value;
}
