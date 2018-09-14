#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "component.h"
#include "enginedefines.h"

struct PRS {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::vec3 eulerAngles;
};

SUEDE_DEFINE_OBJECT_POINTER(GameObject);
SUEDE_DEFINE_OBJECT_POINTER(Transform);

class ITransform : virtual public IComponent {
public:
	typedef SuedeEnumerable<std::vector<Transform>::iterator> Enumerable;

public:
	virtual bool IsAttachedToScene() = 0;

	virtual void AddChild(Transform child) = 0;
	virtual void RemoveChild(Transform child) = 0;
	virtual void RemoveChildAt(uint index) = 0;

	virtual void SetParent(Transform value) = 0;
	virtual Transform GetParent() const = 0;

	virtual glm::vec3 TransformPoint(const glm::vec3& point) = 0;
	virtual glm::vec3 TransformDirection(const glm::vec3& direction) = 0;

	virtual glm::vec3 InverseTransformPoint(const glm::vec3& point) = 0;
	virtual glm::vec3 InverseTransformDirection(const glm::vec3& direction) = 0;

	virtual Transform FindChild(const std::string& path) = 0;

	virtual int GetChildCount() = 0;
	virtual Transform GetChildAt(int i) = 0;
	virtual Enumerable GetChildren() = 0;

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
};
