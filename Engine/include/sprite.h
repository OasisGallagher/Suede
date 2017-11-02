#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "surface.h"
#include "renderer.h"
#include "animation.h"

class ISprite;
typedef std::shared_ptr<ISprite> Sprite;

class ENGINE_EXPORT ISprite : virtual public IObject {
public:
	virtual void SetActive(bool value) = 0;
	virtual bool GetActive() = 0;

	virtual bool LoadModel(const std::string& path) = 0;

	virtual std::string GetName() = 0;
	virtual void SetName(const std::string& value) = 0;

	virtual void SetParent(Sprite value) = 0;
	virtual Sprite GetParent() = 0;

	virtual void AddChild(Sprite child) = 0;
	virtual void RemoveChild(Sprite child) = 0;

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

	virtual void SetSurface(Surface value) = 0;
	virtual Surface GetSurface() = 0;

	virtual void SetRenderer(Renderer value) = 0;
	virtual Renderer GetRenderer() = 0;
};
