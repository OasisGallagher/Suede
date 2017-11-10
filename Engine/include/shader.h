#pragma once
#include <string>

#include "object.h"

class ENGINE_EXPORT IShader : virtual public IObject {
public:
	virtual bool Load(const std::string& path) = 0;
	virtual unsigned GetNativePointer() = 0;

	virtual bool Link() = 0;
};

typedef std::shared_ptr<IShader> Shader;
