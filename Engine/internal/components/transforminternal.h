#include <vector>
#include <ZThread/Mutex.h>

#include "transform.h"
#include "tools/dirtybits.h"
#include "internal/components/componentinternal.h"

class TransformInternal : public ComponentInternal, public DirtyBits {
public:
	static ZThread::Mutex hierarchyMutex;

public:
	TransformInternal();
	~TransformInternal();

public:
	bool IsAttachedToScene(ITransform* transform);

	void AddChild(ITransform* self, Transform child);
	void RemoveChild(Transform child);
	void RemoveChildAt(uint index);

	void SetParent(ITransform* self, Transform value);
	Transform GetParent() const { return parent_; }

	Vector3 TransformPoint(ITransform* self, const Vector3& point);
	Vector3 TransformDirection(ITransform* self, const Vector3& direction);

	Vector3 InverseTransformPoint(ITransform* self, const Vector3& point);
	Vector3 InverseTransformDirection(ITransform* self, const Vector3& direction);

	Transform FindChild(const std::string& path);

	int GetChildCount() { return (int)children_.size(); }
	Transform GetChildAt(int i) { return children_[i]; }
	ITransform::Enumerable GetChildren() { return ITransform::Enumerable(children_.begin(), children_.end()); }

	void SetScale(const Vector3& value);
	void SetPosition(const Vector3& value);
	void SetRotation(const Quaternion& value);
	void SetEulerAngles(const Vector3& value);

	Vector3 GetScale(ITransform* self);
	Vector3 GetPosition(ITransform* self);
	Quaternion GetRotation(ITransform* self);
	Vector3 GetEulerAngles(ITransform* self);

	void SetLocalScale(const Vector3& value);
	void SetLocalPosition(const Vector3& value);
	void SetLocalRotation(const Quaternion& value);
	void SetLocalEulerAngles(const Vector3& value);

	Vector3 GetLocalScale(ITransform* self);
	Vector3 GetLocalPosition(ITransform* self);
	Quaternion GetLocalRotation(ITransform* self);
	Vector3 GetLocalEulerAngles(ITransform* self);

	Matrix4 GetLocalToWorldMatrix(ITransform* self);
	Matrix4 GetWorldToLocalMatrix(ITransform* self);

	Vector3 GetLocalToWorldPosition(ITransform* self, const Vector3& position);
	Vector3 GetWorldToLocalPosition(ITransform* self, const Vector3& position);

	Vector3 GetUp(ITransform* self);
	Vector3 GetRight(ITransform* self);
	Vector3 GetForward(ITransform* self);

	int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	void SetDirty(int bits);

private:
	void DirtyChildrenScales();
	void DirtyChildrenPositions();
	void DirtyChildrenRotationsAndEulerAngles();

	bool IsNullOrRoot(Transform transform);
	Transform FindDirectChild(const std::string& name);
	void ChangeParent(ITransform* self, Transform oldParent, Transform newParent);

	typedef std::vector<Transform> Children;

	bool AddChildItem(Children& children, Transform child);
	bool RemoveChildItem(Children& children, Transform child);

private:
	enum {
		LocalScale = 1,
		WorldScale = 1 << 1,
		LocalRotation = 1 << 2,
		WorldRotation = 1 << 3,
		LocalPosition = 1 << 4,
		WorldPosition = 1 << 5,
		LocalEulerAngles = 1 << 6,
		WorldEulerAngles = 1 << 7,
		LocalToWorldMatrix = 1 << 8,
		WorldToLocalMatrix = 1 << 9,
	};

private:
	Children children_;
	ITransform* parent_;

	PRS local_;
	PRS world_;

	Matrix4 localToWorldMatrix_;
	Matrix4 worldToLocalMatrix_;
};
