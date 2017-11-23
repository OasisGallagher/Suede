#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "material.h"

SUEDE_DEFINE_OBJECT_POINTER(Font);

class IFont : virtual public IObject {
public:
	virtual bool Load(const std::string& path, int size) = 0;
	virtual bool Require(const std::string& str) = 0;
	virtual Material GetMaterial() = 0;
};
