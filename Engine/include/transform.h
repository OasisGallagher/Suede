#pragma once
#include <vector>
#include <functional>

#include "defines.h"
#include "component.h"
#include "math/matrix4.h"

enum class TraversalCommand {
	Break,
	Continue,
	NextSibling,
};

struct PRS {
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Vector3 eulerAngles;
};

class SUEDE_API Transform : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Transform)
	SUEDE_DECLARE_IMPLEMENTATION(Transform)

public:
	Transform();

public:
	bool IsAttachedToScene();
	void TraversalHierarchy(std::function<TraversalCommand(Transform*)> func);

	void AddChild(Transform* child);
	void RemoveChild(Transform* child);
	void RemoveChildAt(uint index);

	void SetParent(Transform* value);
	Transform* GetParent() const;

	Vector3 TransformPoint(const Vector3& point);
	Vector3 TransformDirection(const Vector3& direction);

	Vector3 InverseTransformPoint(const Vector3& point);
	Vector3 InverseTransformDirection(const Vector3& direction);

	Transform* FindChild(const std::string& path);

	int GetChildCount();
	Transform* GetChildAt(int i);

	void LookAt(const Vector3& target, const Vector3& up);

	void SetScale(const Vector3& value);
	void SetPosition(const Vector3& value);
	void SetRotation(const Quaternion& value);
	void SetEulerAngles(const Vector3& value);

	Vector3 GetScale();
	Vector3 GetPosition();
	Quaternion GetRotation();
	Vector3 GetEulerAngles();

	void SetLocalScale(const Vector3& value);
	void SetLocalPosition(const Vector3& value);
	void SetLocalRotation(const Quaternion& value);
	void SetLocalEulerAngles(const Vector3& value);

	Vector3 GetLocalScale();
	Vector3 GetLocalPosition();
	Quaternion GetLocalRotation();
	Vector3 GetLocalEulerAngles();

	Matrix4 GetLocalToWorldMatrix();
	Matrix4 GetWorldToLocalMatrix();

	Vector3 GetLocalToWorldPosition(const Vector3& position);
	Vector3 GetWorldToLocalPosition(const Vector3& position);

	Vector3 GetUp();
	Vector3 GetRight();
	Vector3 GetForward();
};
