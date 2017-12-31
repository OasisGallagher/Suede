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
	virtual Object Clone();
	virtual void SetShader(Shader value);
	virtual Shader GetShader() { return shader_; }

	virtual void SetPass(int value) { pass_ = value; }
	virtual int GetPass() const { return pass_; }
	virtual uint GetPassCount() const;

	virtual void Bind(uint pass);
	virtual void Unbind();

	virtual void Define(const std::string& name);
	virtual void Undefine(const std::string& name);

	virtual void SetInt(const std::string& name, int value);
	virtual void SetFloat(const std::string& name, float value);
	virtual void SetTexture(const std::string& name, Texture value);
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value);
	virtual void SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count);
	virtual void SetVector3(const std::string& name, const glm::vec3& value);
	virtual void SetVector4(const std::string& name, const glm::vec4& value);

	virtual int GetInt(const std::string& name);
	virtual float GetFloat(const std::string& name);
	virtual Texture GetTexture(const std::string& name);
	virtual glm::mat4 GetMatrix4(const std::string& name);
	virtual glm::vec3 GetVector3(const std::string& name);
	virtual glm::vec4 GetVector4(const std::string& name);

private:
	void BindProperties(uint pass);
	void UnbindProperties();

	void AddBuildinProperties();

	// TODO: interface.
	Variant* GetProperty(const std::string& name, VariantType type, bool* newItem = nullptr);

private:
	int pass_;
	Shader shader_;
	uint textureIndex_;
	typedef PtrMap<std::string, Variant> PropertyContainer;
	PropertyContainer properties_;
};
