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

	glm::vec3 TransformPoint(ITransform* self, const glm::vec3& point);
	glm::vec3 TransformDirection(ITransform* self, const glm::vec3& direction);

	glm::vec3 InverseTransformPoint(ITransform* self, const glm::vec3& point);
	glm::vec3 InverseTransformDirection(ITransform* self, const glm::vec3& direction);

	Transform FindChild(const std::string& path);

	int GetChildCount() { return (int)children_.size(); }
	Transform GetChildAt(int i) { return children_[i]; }
	ITransform::Enumerable GetChildren() { return ITransform::Enumerable(children_.begin(), children_.end()); }

	void SetScale(const glm::vec3& value);
	void SetPosition(const glm::vec3& value);
	void SetRotation(const glm::quat& value);
	void SetEulerAngles(const glm::vec3& value);

	glm::vec3 GetScale(ITransform* self);
	glm::vec3 GetPosition(ITransform* self);
	glm::quat GetRotation(ITransform* self);
	glm::vec3 GetEulerAngles(ITransform* self);

	void SetLocalScale(const glm::vec3& value);
	void SetLocalPosition(const glm::vec3& value);
	void SetLocalRotation(const glm::quat& value);
	void SetLocalEulerAngles(const glm::vec3& value);

	glm::vec3 GetLocalScale(ITransform* self);
	glm::vec3 GetLocalPosition(ITransform* self);
	glm::quat GetLocalRotation(ITransform* self);
	glm::vec3 GetLocalEulerAngles(ITransform* self);

	glm::mat4 GetLocalToWorldMatrix(ITransform* self);
	glm::mat4 GetWorldToLocalMatrix(ITransform* self);

	glm::vec3 GetLocalToWorldPosition(ITransform* self, const glm::vec3& position);
	glm::vec3 GetWorldToLocalPosition(ITransform* self, const glm::vec3& position);

	glm::vec3 GetUp(ITransform* self);
	glm::vec3 GetRight(ITransform* self);
	glm::vec3 GetForward(ITransform* self);

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

	glm::mat4 localToWorldMatrix_;
	glm::mat4 worldToLocalMatrix_;
};
