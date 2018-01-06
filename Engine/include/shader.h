#pragma once
#include <string>
#include <vector>

#include "object.h"
#include "variant.h"

struct Property {
	std::string name;
	Variant value;
};

class SUEDE_API IShader : virtual public IObject {
public:
	virtual bool Load(const std::string& path) = 0;

	virtual void Bind(uint ssi, uint pass) = 0;
	virtual void Unbind() = 0;

	virtual void EnablePass(uint ssi, uint pass) = 0;
	virtual void DisablePass(uint ssi, uint pass) = 0;
	virtual bool IsPassEnabled(uint ssi, uint pass) const = 0;

	virtual void EnablePass(uint ssi, const std::string& passName) = 0;
	virtual void DisablePass(uint ssi, const std::string& passName) = 0;
	virtual bool IsPassEnabled(uint ssi, const std::string& passName) const = 0;

	virtual uint GetPassCount(uint ssi) const = 0;
	virtual void GetProperties(std::vector<Property>& properties) = 0;
	virtual bool SetProperty(uint ssi, uint pass, const std::string& name, const void* data) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Shader);
SUEDE_DECLARE_OBJECT_CREATER(Shader);
