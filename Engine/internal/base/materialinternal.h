#pragma once

#include "material.h"
#include "containers/ptrmap.h"
#include "internal/base/objectinternal.h"

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

	virtual void SetRenderQueue(uint value);
	virtual uint GetRenderQueue() const;

	virtual void SetPass(int value) { pass_ = value; }
	virtual int GetPass() const { return pass_; }
	virtual uint GetPassCount() const;
	virtual uint GetPassNativePointer(uint pass) const;

	virtual void Bind(uint pass);
	virtual void Unbind();

	virtual bool EnablePass(uint pass);
	virtual bool DisablePass(uint pass);
	virtual bool IsPassEnabled(uint pass) const;

	virtual int GetPassIndex(const std::string& name) const;

	virtual void Define(const std::string& name);
	virtual void Undefine(const std::string& name);

	virtual void SetInt(const std::string& name, int value);
	virtual void SetFloat(const std::string& name, float value);
	virtual void SetTexture(const std::string& name, Texture value);
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value);
	virtual void SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count);
	virtual void SetVector3(const std::string& name, const glm::vec3& value);
	virtual void SetColor3(const std::string& name, const glm::vec3& value);
	virtual void SetColor4(const std::string& name, const glm::vec4& value);
	virtual void SetVector4(const std::string& name, const glm::vec4& value);

	virtual int GetInt(const std::string& name);
	virtual float GetFloat(const std::string& name);
	virtual Texture GetTexture(const std::string& name);
	virtual glm::mat4 GetMatrix4(const std::string& name);
	virtual glm::vec3 GetVector3(const std::string& name);
	virtual glm::vec3 GetColor3(const std::string& name);
	virtual glm::vec4 GetColor4(const std::string& name);
	virtual glm::vec4 GetVector4(const std::string& name);

	virtual void GetProperties(std::vector<const Property*>& properties) const;

private:
	void BindProperties(uint pass);
	void UnbindProperties();

	void SetVariant(const std::string& name, const Variant& value);

	void InitializeProperties();
	void InitializeEnabledState();

	// TODO: interface.
	Variant* GetProperty(const std::string& name, VariantType type);

private:
	int pass_;
	Shader shader_;
	uint passEnabled_;
	std::string name_;
	typedef ptr_map<std::string, Property> PropertyContainer;
	PropertyContainer properties_;
};
