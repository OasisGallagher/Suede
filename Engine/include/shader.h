#pragma once
#include <string>
#include <vector>

#include "object.h"

enum ShaderPropertyType {
	ShaderPropertyTypeInt,
	ShaderPropertyTypeBool,
	ShaderPropertyTypeFloat,
	ShaderPropertyTypeMatrix4,
	ShaderPropertyTypeMatrix4Array,
	ShaderPropertyTypeVector3,
	ShaderPropertyTypeVector4,
	ShaderPropertyTypeTexture,
};

struct ShaderProperty {
	std::string name;
	ShaderPropertyType type;
};

class SUEDE_API IShader : virtual public IObject {
public:
	virtual bool Load(const std::string& path) = 0;

	virtual void Bind(uint pass) = 0;
	virtual void Unbind() = 0;

	virtual uint GetPassCount() = 0;

	virtual bool SetProperty(const std::string& name, const void* data) = 0;
	virtual void GetProperties(std::vector<ShaderProperty>& properties) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Shader);
SUEDE_DECLARE_OBJECT_CREATER(Shader);
