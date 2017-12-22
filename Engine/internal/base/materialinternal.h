#pragma once

#include "material.h"
#include "internal/containers/ptrmap.h"
#include "internal/containers/variant.h"
#include "internal/base/objectinternal.h"

class RenderState;

class MaterialInternal : public IMaterial, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Material)

public:
	MaterialInternal();
	~MaterialInternal();

public:
	virtual Object Clone();
	virtual void SetShader(Shader value);
	virtual Shader GetShader() { return shader_; }

	virtual void Bind();
	virtual void Unbind();

	virtual void Define(const std::string& name);
	virtual void Undefine(const std::string& name);

	virtual void SetRenderState(RenderStateType type, int parameter0, int parameter1, int parameter2);

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
	void BindProperties();
	void UnbindProperties();

	void BindRenderStates();
	void UnbindRenderStates();

	Variant* GetProperty(const std::string& name, VariantType type);

private:
	Shader shader_;
	int oldProgram_;
	RenderState* states_[RenderStateCount];
	typedef PtrMap<std::string, Variant> PropertyContainer;
	PropertyContainer properties_;
};
