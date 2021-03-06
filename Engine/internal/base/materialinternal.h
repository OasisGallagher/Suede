#pragma once

#include "material.h"
#include "tools/event.h"
#include "globjectmaintainer.h"
#include "containers/ptrmap.h"
#include "internal/base/objectinternal.h"

struct MaterialProperty {
	int mask;
	Property property;
};

class Context;
class MaterialInternal : public ObjectInternal, public GLObjectMaintainer {
public:
	MaterialInternal(Context* context);
	~MaterialInternal();

public:
	ref_ptr<Material> Clone();

	void SetShader(Material* self, Shader* value);
	Shader* GetShader() { return shader_; }

	void SetRenderQueue(int value);
	int GetRenderQueue() const;

	void SetPass(int value) { currentPass_ = value; }
	int GetActivatedPass() const { return activatedPass_; }
	uint GetPassCount() const;

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
	void SetTexture(const std::string& name, Texture* value);
	void SetMatrix4(const std::string& name, const Matrix4& value);
	void SetMatrix4Array(const std::string& name, const Matrix4* ptr, uint count);
	void SetVector2(const std::string& name, const Vector2& value);
	void SetVector3(const std::string& name, const Vector3& value);
	void SetVector3Array(const std::string& name, const Vector3* ptr, uint count);
	void SetColor(const std::string& name, const Color& value);
	void SetVector4(const std::string& name, const Vector4& value);
	void SetVariant(const std::string& name, const Variant& value);

	int GetInt(const std::string& name);
	bool GetBool(const std::string& name);
	iranged GetRangedInt(const std::string& name);
	franged GetRangedFloat(const std::string& name);
	float GetFloat(const std::string& name);
	Texture* GetTexture(const std::string& name);
	Matrix4 GetMatrix4(const std::string& name);
	Vector2 GetVector2(const std::string& name);
	Vector3 GetVector3(const std::string& name);
	Color GetColor(const std::string& name);
	Vector4 GetVector4(const std::string& name);

	const std::vector<const Property*>& GetExplicitProperties();

public:
	static event<MaterialInternal*> shaderChanged;

protected:
	virtual void OnContextDestroyed();

private:
	void ApplyShader();

	void BindProperties(uint pass);
	void UnbindProperties();

	void CopyProperties(Shader* newShader);
	void AddRedundantProperty(const std::string& name, const Variant& value);
	void DeactiveRedundantProperties(const std::vector<ShaderProperty>& shaderProperties);

	void InitializeEnabledState();

	Variant* GetProperty(const std::string& name, VariantType type);
	MaterialProperty* GetMaterialProperty(const std::string& name, VariantType type);

	Variant* VerifyProperty(const std::string& name, VariantType type);

private:
	Shader* shader_ = nullptr;
	bool shaderDirty_ = false;

	int currentPass_ = -1;
	int activatedPass_ = -1;

	uint passEnabled_ = 0;

	ptr_map<std::string, MaterialProperty> properties_;
	std::vector<const Property*> explicitProperties_;
};
