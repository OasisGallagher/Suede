#pragma once
#include <string>
#include <vector>

#include "object.h"
#include "variant.h"

struct Property {
	std::string name;
	Variant value;
};

struct ShaderProperty {
	int mask;
	Property* property;
};

class SUEDE_API IShader : virtual public IObject {
public:
	virtual std::string GetName() const = 0;
	virtual bool Load(const std::string& path) = 0;

	virtual void Bind(uint ssi, uint pass) = 0;
	virtual void Unbind() = 0;

	virtual void SetRenderQueue(uint ssi, int value) = 0;
	virtual int GetRenderQueue(uint ssi) const = 0;

	/**
	 * @return initial enabled state of pass `pass`.
	 */
	virtual bool IsPassEnabled(uint ssi, uint pass) const = 0;
	virtual int GetPassIndex(uint ssi, const std::string& name) const = 0;
	virtual uint GetNativePointer(uint ssi, uint pass) const = 0;

	virtual uint GetPassCount(uint ssi) const = 0;
	virtual uint GetSubShaderCount() const = 0;

	/**
	* @param properties return initial properties defined in shader.
	*/
	virtual void GetProperties(std::vector<ShaderProperty>& properties) = 0;
	virtual bool SetProperty(uint ssi, uint pass, const std::string& name, const void* data) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Shader);
SUEDE_DECLARE_OBJECT_CREATER(Shader);
