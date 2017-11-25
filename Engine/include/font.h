#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "material.h"

SUEDE_DEFINE_OBJECT_POINTER(Font);

class SUEDE_API IFont : virtual public IObject {
public:
	virtual bool Load(const std::string& path, int size) = 0;
	virtual bool Require(const std::wstring& str) = 0;
	virtual Material GetMaterial() = 0;
	virtual glm::vec4 GetTexCoord(wchar_t wch) = 0;
};
