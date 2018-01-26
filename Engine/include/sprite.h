#pragma once
#include <glm/glm.hpp>

#include "mesh.h"
#include "renderer.h"
#include "animation.h"

SUEDE_DEFINE_OBJECT_POINTER(Sprite);
SUEDE_DECLARE_OBJECT_CREATER(Sprite);

class SUEDE_API ISprite : virtual public IObject {
public:
	virtual bool GetActive() const = 0;

	virtual void SetActiveSelf(bool value) = 0;
	virtual bool GetActiveSelf() const = 0;

	virtual const std::string& GetTag() const = 0;
	virtual bool SetTag(const std::string& value) = 0;

	virtual std::string GetName() const = 0;
	virtual void SetName(const std::string& value) = 0;

	virtual void SetParent(Sprite value) = 0;
	virtual Sprite GetParent() const = 0;

	virtual Sprite FindChild(const std::string& path) = 0;

	virtual void AddChild(Sprite child) = 0;
	virtual void RemoveChild(Sprite child) = 0;
	virtual void RemoveChildAt(uint index) = 0;

	virtual int GetChildCount() = 0;
	virtual Sprite GetChildAt(int i) = 0;

	virtual void SetScale(const glm::vec3& value) = 0;
	virtual void SetPosition(const glm::vec3& value) = 0;
	virtual void SetRotation(const glm::quat& value) = 0;
	virtual void SetEulerAngles(const glm::vec3& value) = 0;

	virtual glm::vec3 GetScale() = 0;
	virtual glm::vec3 GetPosition() = 0;
	virtual glm::quat GetRotation() = 0;
	virtual glm::vec3 GetEulerAngles() = 0;

	virtual void SetLocalScale(const glm::vec3& value) = 0;
	virtual void SetLocalPosition(const glm::vec3& value) = 0;
	virtual void SetLocalRotation(const glm::quat& value) = 0;
	virtual void SetLocalEulerAngles(const glm::vec3& value) = 0;

	virtual glm::vec3 GetLocalScale() = 0;
	virtual glm::vec3 GetLocalPosition() = 0;
	virtual glm::quat GetLocalRotation() = 0;
	virtual glm::vec3 GetLocalEulerAngles() = 0;

	virtual glm::mat4 GetLocalToWorldMatrix() = 0;
	virtual glm::mat4 GetWorldToLocalMatrix() = 0;

	virtual glm::vec3 GetLocalToWorldPosition(const glm::vec3& position) = 0;
	virtual glm::vec3 GetWorldToLocalPosition(const glm::vec3& position) = 0;

	virtual glm::vec3 GetUp() = 0;
	virtual glm::vec3 GetRight() = 0;
	virtual glm::vec3 GetForward() = 0;

	virtual void Update() = 0;

	virtual void SetAnimation(Animation value) = 0;
	virtual Animation GetAnimation() = 0;

	virtual void SetMesh(Mesh value) = 0;
	virtual Mesh GetMesh() = 0;

	virtual void SetRenderer(Renderer value) = 0;
	virtual Renderer GetRenderer() = 0;
};
