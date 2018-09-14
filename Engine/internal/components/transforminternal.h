#include <vector>
#include <ZThread/Mutex.h>

#include "transform.h"
#include "tools/dirtybits.h"
#include "internal/components/componentinternal.h"

class TransformInternal : virtual public ITransform, public ComponentInternal, public DirtyBits {
	DEFINE_FACTORY_METHOD(Transform)

public:
	static ZThread::Mutex hierarchyMutex;

public:
	TransformInternal();
	~TransformInternal();

public:
	virtual bool IsAttachedToScene();

	virtual void AddChild(Transform child);
	virtual void RemoveChild(Transform child);
	virtual void RemoveChildAt(uint index);

	virtual void SetParent(Transform value);
	virtual Transform GetParent() const { return parent_.lock(); }

	virtual glm::vec3 TransformPoint(const glm::vec3& point);
	virtual glm::vec3 TransformDirection(const glm::vec3& direction);

	virtual glm::vec3 InverseTransformPoint(const glm::vec3& point);
	virtual glm::vec3 InverseTransformDirection(const glm::vec3& direction);

	virtual Transform FindChild(const std::string& path);

	virtual int GetChildCount() { return (int)children_.size(); }
	virtual Transform GetChildAt(int i) { return children_[i]; }
	virtual Enumerable GetChildren() { return Enumerable(children_.begin(), children_.end()); }

	virtual void SetScale(const glm::vec3& value);
	virtual void SetPosition(const glm::vec3& value);
	virtual void SetRotation(const glm::quat& value);
	virtual void SetEulerAngles(const glm::vec3& value);

	virtual glm::vec3 GetScale();
	virtual glm::vec3 GetPosition();
	virtual glm::quat GetRotation();
	virtual glm::vec3 GetEulerAngles();

	virtual void SetLocalScale(const glm::vec3& value);
	virtual void SetLocalPosition(const glm::vec3& value);
	virtual void SetLocalRotation(const glm::quat& value);
	virtual void SetLocalEulerAngles(const glm::vec3& value);

	virtual glm::vec3 GetLocalScale();
	virtual glm::vec3 GetLocalPosition();
	virtual glm::quat GetLocalRotation();
	virtual glm::vec3 GetLocalEulerAngles();

	virtual glm::mat4 GetLocalToWorldMatrix();
	virtual glm::mat4 GetWorldToLocalMatrix();

	virtual glm::vec3 GetLocalToWorldPosition(const glm::vec3& position);
	virtual glm::vec3 GetWorldToLocalPosition(const glm::vec3& position);

	virtual glm::vec3 GetUp();
	virtual glm::vec3 GetRight();
	virtual glm::vec3 GetForward();

	virtual int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	virtual void SetDirty(int bits);

private:
	void DirtyChildrenScales();
	void DirtyChildrenPositions();
	void DirtyChildrenRotationsAndEulerAngles();

	bool IsNullOrRoot(Transform transform);
	Transform FindDirectChild(const std::string& name);
	void ChangeParent(Transform oldParent, Transform newParent);

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
