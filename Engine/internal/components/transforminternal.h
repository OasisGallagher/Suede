#include <vector>

#include "transform.h"
#include "tools/event.h"
#include "internal/components/componentinternal.h"

class TransformInternal : public ComponentInternal {
public:
	TransformInternal();
	~TransformInternal();

public:
	static event<Transform*, bool> attached;

public:
	bool IsAttachedToScene(Transform* transform);

	void AddChild(Transform* self, Transform* child);
	void RemoveChild(Transform* child);
	void RemoveChildAt(uint index);

	void SetParent(Transform* self, Transform* value);
	Transform* GetParent() const { return parent_; }

	Vector3 TransformPoint(Transform* self, const Vector3& point);
	Vector3 TransformDirection(Transform* self, const Vector3& direction);

	Vector3 InverseTransformPoint(Transform* self, const Vector3& point);
	Vector3 InverseTransformDirection(Transform* self, const Vector3& direction);

	Transform* FindChild(const std::string& path);

	int GetChildCount() { return (int)children_.size(); }
	Transform* GetChildAt(int i) { return children_[i].get(); }

	void LookAt(Transform* self, const Vector3& target, const Vector3& up);

	void SetScale(const Vector3& value);
	void SetPosition(const Vector3& value);
	void SetRotation(const Quaternion& value);
	void SetEulerAngles(const Vector3& value);

	Vector3 GetScale(Transform* self);
	Vector3 GetPosition(Transform* self);
	Quaternion GetRotation(Transform* self);
	Vector3 GetEulerAngles(Transform* self);

	void SetLocalScale(const Vector3& value);
	void SetLocalPosition(const Vector3& value);
	void SetLocalRotation(const Quaternion& value);
	void SetLocalEulerAngles(const Vector3& value);

	Vector3 GetLocalScale(Transform* self);
	Vector3 GetLocalPosition(Transform* self);
	Quaternion GetLocalRotation(Transform* self);
	Vector3 GetLocalEulerAngles(Transform* self);

	Matrix4 GetLocalToWorldMatrix(Transform* self);
	Matrix4 GetWorldToLocalMatrix(Transform* self);

	Vector3 GetLocalToWorldPosition(Transform* self, const Vector3& position);
	Vector3 GetWorldToLocalPosition(Transform* self, const Vector3& position);

	Vector3 GetUp(Transform* self);
	Vector3 GetRight(Transform* self);
	Vector3 GetForward(Transform* self);

	int GetUpdateStrategy() { return UpdateStrategyNone; }

private:
	void DirtyChildrenScales();
	void DirtyChildrenPositions();
	void DirtyChildrenRotationsAndEulerAngles();

	bool IsNullOrRoot(Transform* transform);
	Transform* FindDirectChild(const std::string& name);
	void ChangeParent(Transform* self, Transform* oldParent, Transform* newParent);

	typedef std::vector<ref_ptr<Transform>> Children;

	bool AddChildItem(Children& children, Transform* child);
	bool RemoveChildItem(Children& children, Transform* child);

private:
	enum {
		LocalScaleMask = 1,
		WorldScaleMask = 1 << 1,
		LocalRotationMask = 1 << 2,
		WorldRotationMask = 1 << 3,
		LocalPositionMask = 1 << 4,
		WorldPositionMask = 1 << 5,
		LocalEulerAnglesMask = 1 << 6,
		WorldEulerAnglesMask = 1 << 7,
		LocalToWorldMatrixMask = 1 << 8,
		WorldToLocalMatrixMask = 1 << 9,
	};

	void SetDirty(uint bits, bool value);
	bool IsDirty(uint bits) const { return (dirtyBits_ & bits) != 0; }

private:
	uint dirtyBits_ = 0;
	Children children_;
	Transform* parent_ = nullptr;

	PRS local_;
	PRS world_;

	Matrix4 localToWorldMatrix_;
	Matrix4 worldToLocalMatrix_;
};
