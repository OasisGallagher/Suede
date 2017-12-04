#pragma once
#include <string>

#include "object.h"

class SUEDE_API IShader : virtual public IObject {
public:
	virtual bool Load(const std::string& path) = 0;
	virtual uint GetNativePointer() = 0;

	virtual bool Link() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Shader);
SUEDE_DECLARE_OBJECT_CREATER(Shader);
