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

class SUEDE_API Shader : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Shader)
	SUEDE_DECLARE_IMPLEMENTATION(Shader)

public:
	static Shader* Find(const std::string& path);

public:
	std::string GetName() const;

	void Bind(uint ssi, uint pass);
	void Unbind();

	void SetRenderQueue(uint ssi, int value);
	int GetRenderQueue(uint ssi) const;

	/**
	 * @return initial enabled state of pass `pass`.
	 */
	bool IsPassEnabled(uint ssi, uint pass) const;
	int GetPassIndex(uint ssi, const std::string& name) const;
	uint GetNativePointer(uint ssi, uint pass) const;

	uint GetPassCount(uint ssi) const;
	uint GetSubShaderCount() const;

	/**
	* @param properties return initial properties defined in shader.
	*/
	void GetProperties(std::vector<ShaderProperty>& properties);
	bool SetProperty(uint ssi, uint pass, const std::string& name, const void* data);

private:
	Shader();
};
