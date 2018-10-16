#include <vector>
#include <ZThread/Mutex.h>

#include "transform.h"
#include "tools/dirtybits.h"
#include "internal/components/componentinternal.h"

class TransformInternal : public ComponentInternal, public DirtyBits {
	DEFINE_FACTORY_METHOD(Transform)

public:
	static ZThread::Mutex hierarchyMutex;

public:
	TransformInternal();
	~TransformInternal();

public:
	bool IsAttachedToScene(Transform transform);

	void AddChild(Transform self, Transform child);
	void RemoveChild(Transform child);
	void RemoveChildAt(uint index);

	void SetParent(Transform self, Transform value);
	Transform GetParent() const { return parent_.lock(); }

	glm::vec3 TransformPoint(Transform self, const glm::vec3& point);
	glm::vec3 TransformDirection(Transform self, const glm::vec3& direction);

	glm::vec3 InverseTransformPoint(Transform self, const glm::vec3& point);
	glm::vec3 InverseTransformDirection(Transform self, const glm::vec3& direction);

	Transform FindChild(const std::string& path);

	int GetChildCount() { return (int)children_.size(); }
	Transform GetChildAt(int i) { return children_[i]; }
	ITransform::Enumerable GetChildren() { return ITransform::Enumerable(children_.begin(), children_.end()); }

	void SetScale(const glm::vec3& value);
	void SetPosition(const glm::vec3& value);
	void SetRotation(const glm::quat& value);
	void SetEulerAngles(const glm::vec3& value);

	glm::vec3 GetScale(Transform self);
	glm::vec3 GetPosition(Transform self);
	glm::quat GetRotation(Transform self);
	glm::vec3 GetEulerAngles(Transform self);

	void SetLocalScale(const glm::vec3& value);
	void SetLocalPosition(const glm::vec3& value);
	void SetLocalRotation(const glm::quat& value);
	void SetLocalEulerAngles(const glm::vec3& value);

	glm::vec3 GetLocalScale(Transform self);
	glm::vec3 GetLocalPosition(Transform self);
	glm::quat GetLocalRotation(Transform self);
	glm::vec3 GetLocalEulerAngles(Transform self);

	glm::mat4 GetLocalToWorldMatrix(Transform self);
	glm::mat4 GetWorldToLocalMatrix(Transform self);

	glm::vec3 GetLocalToWorldPosition(Transform self, const glm::vec3& position);
	glm::vec3 GetWorldToLocalPosition(Transform self, const glm::vec3& position);

	glm::vec3 GetUp(Transform self);
	glm::vec3 GetRight(Transform self);
	glm::vec3 GetForward(Transform self);

	int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	void SetDirty(int bits);

private:
	void DirtyChildrenScales();
	void DirtyChildrenPositions();
	void DirtyChildrenRotationsAndEulerAngles();

	bool IsNullOrRoot(Transform transform);
	Transform FindDirectChild(const std::string& name);
	void ChangeParent(Transform self, Transform oldParent, Transform newParent);

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
	std::weak_ptr<Transform::element_type> parent_;

	PRS local_;
	PRS world_;

	glm::mat4 localToWorldMatrix_;
	glm::mat4 worldToLocalMatrix_;
};
