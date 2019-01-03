#pragma once

#include "material.h"
#include "containers/ptrmap.h"
#include "internal/base/objectinternal.h"

struct MaterialProperty {
	int i, mask;
	Property property;
};

class MaterialInternal : public ObjectInternal {
public:
	MaterialInternal();
	~MaterialInternal();

public:
	virtual Object Clone();

public:
	void SetShader(IMaterial* self, Shader value);
	Shader GetShader() { return shader_; }

	void SetRenderQueue(int value);
	int GetRenderQueue() const;

	void SetPass(int value) { currentPass_ = value; }
	int GetPass() const { return currentPass_; }
	uint GetPassCount() const;
	uint GetPassNativePointer(uint pass) const;

	void Bind(uint pass);
	void Unbind();

	bool EnablePass(uint pass);
	bool DisablePass(uint pass);
	bool IsPassEnabled(uint pass) const;

	int FindPass(const std::string& name) const;

	void Define(const std::string& name);
	void Undefine(const std::string& name);

	bool HasProperty(const std::string& name) const { return properties_.contains(name); }

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
	iranged GetRangedInt(const std::string& name);
	franged GetRangedFloat(const std::string& name);
	float GetFloat(const std::string& name);
	Texture GetTexture(const std::string& name);
	glm::mat4 GetMatrix4(const std::string& name);
	glm::vec3 GetVector3(const std::string& name);
	Color GetColor(const std::string& name);
	glm::vec4 GetVector4(const std::string& name);

	const std::vector<const Property*>& GetExplicitProperties();

private:
	void BindProperties(uint pass);
	void UnbindProperties();

	void UpdateProperties(IMaterial* self, Shader newShader);
	void CopyProperties(Shader newShader);
	void DeactiveRedundantProperties(const std::vector<ShaderProperty>& shaderProperties);

	void InitializeEnabledState();

	Variant* GetProperty(const std::string& name, VariantType type);
	MaterialProperty* GetMaterialProperty(const std::string& name, VariantType type);

	Variant* VerifyProperty(const std::string& name, VariantType type);

private:
	Shader shader_;
	int currentPass_;

	uint passEnabled_;

	typedef ptr_map<std::string, MaterialProperty> PropertyContainer;
	PropertyContainer properties_;
	std::vector<const Property*> explicitProperties_;
};
