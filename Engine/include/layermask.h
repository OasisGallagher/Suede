#pragma once
#include <string>

#include "types.h"
#include "subsystem.h"

class SUEDE_API LayerMask : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(LayerMask)
public:
	enum {
		SystemType = SubsystemType::LayerMask,
	};

public:
	LayerMask();

public:
	int GetMask(std::initializer_list<std::string> names);
	int GetAllLayers();

	/**
	 * Given a layer index, returns the name of the layer as defined in either a Builtin or a User Layer in the Tags and Layers manager.
	 */
	std::string LayerToName(int layerIndex);

	/**
	 * Given a layer name, returns the layer index as defined by either a Builtin or a User Layer in the Tags and Layers manager.
	 */
	int NameToLayer(const std::string& name);
};
