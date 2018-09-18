#pragma once

#include "material.h"
#include "containers/ptrmap.h"
#include "internal/base/objectinternal.h"

struct MaterialProperty {
	MaterialProperty& operator = (const ShaderProperty& p);

	int mask;
	Property property;
};

class MaterialInternal : public IMaterial, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Material)

public:
	MaterialInternal();
	~MaterialInternal();

public:
	virtual void SetName(const std::string& value) { name_ = value; }
	virtual const std::string& GetName() const { return name_; }

	virtual Object Clone();

	virtual void SetShader(Shader value);
	virtual Shader GetShader() { return shader_; }

	virtual void SetRenderQueue(int value);
	virtual int GetRenderQueue() const;

	virtual void SetPass(int value) { currentPass_ = value; }
	virtual int GetPass() const { return currentPass_; }
	virtual uint GetPassCount() const;
	virtual uint GetPassNativePointer(uint pass) const;

	virtual void Bind(uint pass);
	virtual void Unbind();

	virtual bool EnablePass(uint pass);
	virtual bool DisablePass(uint pass);
	virtual bool IsPassEnabled(uint pass) const;

	virtual int FindPass(const std::string& name) const;

	virtual void Define(const std::string& name);
	virtual void Undefine(const std::string& name);

	virtual void SetInt(const std::string& name, int value);
	virtual void SetBool(const std::string& name, bool value);
	virtual void SetFloat(const std::string& name, float value);
	virtual void SetTexture(const std::string& name, Texture value);
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value);
	virtual void SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count);
	virtual void SetVector3(const std::string& name, const glm::vec3& value);
	virtual void SetVector3Array(const std::string& name, const glm::vec3* ptr, uint count);
	virtual void SetColor(const std::string& name, const Color& value);
	virtual void SetVector4(const std::string& name, const glm::vec4& value);
	virtual void SetVariant(const std::string& name, const Variant& value);

	virtual int GetInt(const std::string& name);
	virtual bool GetBool(const std::string& name);
	virtual float GetFloat(const std::string& name);
	virtual Texture GetTexture(const std::string& name);
	virtual glm::mat4 GetMatrix4(const std::string& name);
	virtual glm::vec3 GetVector3(const std::string& name);
	virtual Color GetColor(const std::string& name);
	virtual glm::vec4 GetVector4(const std::string& name);

	virtual void GetProperties(std::vector<const Property*>& properties) const;

private:
	void BindProperties(uint pass);
	void UnbindProperties();

	void UpdateProperties(Shader newShader);
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

	std::string name_;

	typedef ptr_map<std::string, MaterialProperty> PropertyContainer;
	PropertyContainer properties_;
};
