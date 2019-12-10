#pragma once
#include <vector>

#include "component.h"
#include "enginedefines.h"

struct PRS {
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Vector3 eulerAngles;
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

	Vector3 TransformPoint(const Vector3& point);
	Vector3 TransformDirection(const Vector3& direction);

	Vector3 InverseTransformPoint(const Vector3& point);
	Vector3 InverseTransformDirection(const Vector3& direction);

	Transform FindChild(const std::string& path);

	int GetChildCount();
	Transform GetChildAt(int i);
	Enumerable GetChildren();

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
