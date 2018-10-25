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

SUEDE_DEFINE_OBJECT_POINTER(GameObject)
SUEDE_DEFINE_OBJECT_POINTER(Transform)

class SUEDE_API ITransform : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Transform)
	SUEDE_DECLARE_IMPLEMENTATION(Transform)

public:
	ITransform();

public:
	typedef SuedeEnumerable<std::vector<Transform>::iterator> Enumerable;

public:
	bool IsAttachedToScene();

	void AddChild(Transform child);
	void RemoveChild(Transform child);
	void RemoveChildAt(uint index);

	void SetParent(Transform value);
	Transform GetParent() const;

	glm::vec3 TransformPoint(const glm::vec3& point);
	glm::vec3 TransformDirection(const glm::vec3& direction);

	glm::vec3 InverseTransformPoint(const glm::vec3& point);
	glm::vec3 InverseTransformDirection(const glm::vec3& direction);

	Transform FindChild(const std::string& path);

	int GetChildCount();
	Transform GetChildAt(int i);
	Enumerable GetChildren();

	void SetScale(const glm::vec3& value);
	void SetPosition(const glm::vec3& value);
	void SetRotation(const glm::quat& value);
	void SetEulerAngles(const glm::vec3& value);

	glm::vec3 GetScale();
	glm::vec3 GetPosition();
	glm::quat GetRotation();
	glm::vec3 GetEulerAngles();

	void SetLocalScale(const glm::vec3& value);
	void SetLocalPosition(const glm::vec3& value);
	void SetLocalRotation(const glm::quat& value);
	void SetLocalEulerAngles(const glm::vec3& value);

	glm::vec3 GetLocalScale();
	glm::vec3 GetLocalPosition();
	glm::quat GetLocalRotation();
	glm::vec3 GetLocalEulerAngles();

	glm::mat4 GetLocalToWorldMatrix();
	glm::mat4 GetWorldToLocalMatrix();

	glm::vec3 GetLocalToWorldPosition(const glm::vec3& position);
	glm::vec3 GetWorldToLocalPosition(const glm::vec3& position);

	glm::vec3 GetUp();
	glm::vec3 GetRight();
	glm::vec3 GetForward();
};
