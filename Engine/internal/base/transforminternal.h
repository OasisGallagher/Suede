#include <vector>

#include "entity.h"
#include "transform.h"
#include "internal/base/objectinternal.h"

class TransformInternal : public ITransform, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Transform)

public:
	TransformInternal();

public:
	virtual Entity GetEntity() { return entity_.lock(); }
	virtual void SetEntity(Entity value);

	virtual void AddChild(Transform child);
	virtual void RemoveChild(Transform child);
	virtual void RemoveChildAt(uint index);

	virtual void SetParent(Transform value);
	virtual Transform GetParent() const { return parent_.lock(); }

	virtual Transform FindChild(const std::string& path);

	virtual int GetChildCount() { return (int)children_.size(); }
	virtual Transform GetChildAt(int i) { return children_[i]; }

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

private:
	void SetDiry(int bits);
	bool IsDirty(int bits) const { return (dirtyFlag_ & bits) != 0; }
	void ClearDirty(int bits) { dirtyFlag_ &= ~bits; }

	void DirtyChildrenScales();
	void DirtyChildrenPositions();
	void DirtyChildrenRotationsAndEulerAngles();

	glm::mat4 Concatenate(const glm::vec3& t, const glm::quat& r, const glm::vec3& s);

	Transform FindDirectChild(const std::string& name);

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
	std::vector<Transform> children_;
	std::weak_ptr<Transform::element_type> parent_;
	std::weak_ptr<Entity::element_type> entity_;

	TRS local_;
	TRS world_;

	int dirtyFlag_;
	glm::mat4 localToWorldMatrix_;
	glm::mat4 worldToLocalMatrix_;
};
