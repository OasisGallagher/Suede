#pragma once
#include "color.h"
#include "object.h"
#include "shader.h"
#include "variant.h"
#include "texture.h"

enum class RenderQueue {
	Background = 1000,
	Geometry = 2000,
	Transparent = 4000,
	Overlay = 6000,
};

class SUEDE_API Material : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Material)
	SUEDE_DECLARE_IMPLEMENTATION(Material)

public:
	Material();

public:
	ref_ptr<Object> Clone();

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

	void SetShader(Shader* value);
	Shader* GetShader();

	void SetRenderQueue(int value);
	int GetRenderQueue() const;

	void Define(const std::string& name);
	void Undefine(const std::string& name);

	bool HasProperty(const std::string& name) const;

	void SetInt(const std::string& name, int value);
	void SetBool(const std::string& name, bool value);
	void SetFloat(const std::string& name, float value);
	void SetTexture(const std::string& name, Texture* value);
	void SetMatrix4(const std::string& name, const Matrix4& value);
	void SetMatrix4Array(const std::string& name, const Matrix4* ptr, uint count);
	void SetVector3(const std::string& name, const Vector3& value);
	void SetVector3Array(const std::string& name, const Vector3* ptr, uint count);
	void SetColor(const std::string& name, const Color& value);
	void SetVector4(const std::string& name, const Vector4& value);
	void SetVariant(const std::string& name, const Variant& value);

	int GetInt(const std::string& name);
	bool GetBool(const std::string& name);
	float GetFloat(const std::string& name);
	iranged GetRangedInt(const std::string& name);
	franged GetRangedFloat(const std::string& name);
	Texture* GetTexture(const std::string& name);
	Matrix4 GetMatrix4(const std::string& name);
	Vector3 GetVector3(const std::string& name);
	Color GetColor(const std::string& name);
	Vector4 GetVector4(const std::string& name);

	/**
	 * @brief get explicit properties defined in Properties block.
	 */
	const std::vector<const Property*>& GetExplicitProperties();
};
