#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "shader.h"
#include "variant.h"
#include "texture.h"

enum RenderQueue {
	RenderQueueBackground = 1000,
	RenderQueueGeometry = 2000,
	RenderQueueTransparent = 4000,
	RenderQueueOverlay = 6000,
};

class SUEDE_API IMaterial : virtual public IObject {
public:
	virtual void SetName(const std::string& value) = 0;
	virtual const std::string& GetName() const = 0;

	virtual void Bind(uint pass) = 0;
	virtual void Unbind() = 0;

	virtual bool EnablePass(uint pass) = 0;
	virtual bool DisablePass(uint pass) = 0;
	virtual bool IsPassEnabled(uint pass) const = 0;

	virtual int FindPass(const std::string& name) const = 0;

	virtual void SetPass(int pass) = 0;
	virtual int GetPass() const = 0;
	virtual uint GetPassCount() const = 0;
	virtual uint GetPassNativePointer(uint pass) const = 0;

	virtual void SetShader(Shader value) = 0;
	virtual Shader GetShader() = 0;

	virtual void SetRenderQueue(uint value) = 0;
	virtual uint GetRenderQueue() const = 0;

	virtual void Define(const std::string& name) = 0;
	virtual void Undefine(const std::string& name) = 0;

	virtual void SetIntImmediate(uint pass, const std::string& name, int value) = 0;

	virtual void SetInt(const std::string& name, int value) = 0;
	virtual void SetFloat(const std::string& name, float value) = 0;
	virtual void SetTexture(const std::string& name, Texture value) = 0;
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value) = 0;
	virtual void SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) = 0;
	virtual void SetVector3(const std::string& name, const glm::vec3& value) = 0;
	virtual void SetColor3(const std::string& name, const glm::vec3& value) = 0;
	virtual void SetColor4(const std::string& name, const glm::vec4& value) = 0;
	virtual void SetVector4(const std::string& name, const glm::vec4& value) = 0;
	virtual void SetVariant(const std::string& name, const Variant& value) = 0;

	virtual int GetInt(const std::string& name) = 0;
	virtual float GetFloat(const std::string& name) = 0;
	virtual Texture GetTexture(const std::string& name) = 0;
	virtual glm::mat4 GetMatrix4(const std::string& name) = 0;
	virtual glm::vec3 GetVector3(const std::string& name) = 0;
	virtual glm::vec3 GetColor3(const std::string& name) = 0;
	virtual glm::vec4 GetColor4(const std::string& name) = 0;
	virtual glm::vec4 GetVector4(const std::string& name) = 0;

	virtual void GetProperties(std::vector<const Property*>& properties) const = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Material);
SUEDE_DECLARE_OBJECT_CREATER(Material);

