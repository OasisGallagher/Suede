#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "material.h"

DEFINE_OBJECT_PTR(Font);

struct Glyph {
	glm::vec2 lbTexCoord;
	glm::vec2 rtTexCoord;
};

class IFont : virtual public IObject {
public:
	virtual bool Load(const std::string& fname, int size) = 0;
	virtual bool Require(const std::string& str) = 0;
	virtual Material GetMaterial() = 0;
};
