#pragma once
#include <glm/glm.hpp>

#include "color.h"
#include "object.h"
#include "shader.h"
#include "variant.h"
#include "texture.h"

BETTER_ENUM(RenderQueue, int,
	Background = 1000,
	Geometry = 2000,
	Transparent = 4000,
	Overlay = 6000
)

class SUEDE_API IMaterial : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(Material)
	SUEDE_DECLARE_IMPLEMENTATION(Material)

public:
	IMaterial();

public:
	Object Clone();

	void Bind(uint pass);
	void Unbind();

	bool EnablePass(uint pass);
	bool DisablePass(uint pass);
	bool IsPassEnabled(uint pass) const;

	int FindPass(const std::string& name) const;

	void SetPass(int pass);
	int GetPass() const;
	uint GetPassCount() const;
	uint GetPassNativePointer(uint pass) const;

	void SetShader(Shader value);
	Shader GetShader();

	void SetRenderQueue(int value);
	int GetRenderQueue() const;

	void Define(const std::string& name);
	void Undefine(const std::string& name);

	bool HasProperty(const std::string& name) const;

	void SetInt(const std::string& name, int value);
	void SetBool(const std::string& name, bool value);
	void SetFloat(const std::string& name, float value);
	void SetTexture(const std::string& name, Texture value);
	void SetMatrix4(const std::string& name, const glm::mat4& value);
	void SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count);
	void SetVector3(const std::string& name, const glm::vec3& value);
	void SetVector3Array(const std::string& name, const glm::vec3* ptr, uint count);
	void SetColor(const std::string& name, const Color& value);
	void SetVector4(const std::string& name, const glm::vec4& value);
	void SetVariant(const std::string& name, const Variant& value);

	int GetInt(const std::string& name);
	bool GetBool(const std::string& name);
	float GetFloat(const std::string& name);
	irange GetIntRange(const std::string& name);
	frange GetFloatRange(const std::string& name);
	Texture GetTexture(const std::string& name);
	glm::mat4 GetMatrix4(const std::string& name);
	glm::vec3 GetVector3(const std::string& name);
	Color GetColor(const std::string& name);
	glm::vec4 GetVector4(const std::string& name);

	/**
	 * @brief get explicit properties defined in Properties block.
	 */
	const std::vector<const Property*>& GetExplicitProperties();
};

SUEDE_DEFINE_OBJECT_POINTER(Material)
