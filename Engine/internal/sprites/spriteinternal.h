#pragma once
#include <glm/gtc/quaternion.hpp>

#include "mesh.h"
#include "sprite.h"
#include "animation.h"
#include "internal/base/objectinternal.h"

class SpriteInternal : virtual public ISprite, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Sprite)

public:
	SpriteInternal();

public:
	virtual void SetActive(bool value) { active_ = value; }
	virtual bool GetActive() { return active_; }

	virtual const std::string& GetTag() const { return tag_; }
	virtual bool SetTag(const std::string& value);

	virtual std::string GetName() { return name_; }
	virtual void SetName(const std::string& value) { name_ = value; }

	virtual void AddChild(Sprite child);
	virtual void RemoveChild(Sprite child);
	virtual void RemoveChildAt(uint index);

	virtual void SetParent(Sprite value);
	virtual Sprite GetParent() { return parent_.lock(); }

	virtual Sprite FindChild(const std::string& path);

	virtual int GetChildCount() { return (int)children_.size(); }
	virtual Sprite GetChildAt(int i) { return children_[i]; }

public:
	virtual void Update();
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

	virtual void SetAnimation(Animation value) { animation_ = value; }
	virtual Animation GetAnimation() { return animation_; }

	virtual void SetMesh(Mesh value) { mesh_ = value; }
	virtual Mesh GetMesh() { return mesh_; }

	virtual void SetRenderer(Renderer value) { renderer_ = value; }
	virtual Renderer GetRenderer() { return renderer_; }

protected:
	SpriteInternal(ObjectType spriteType);

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
	void SetDiry(int bits);
	bool IsDirty(int bits) const { return (dirtyFlag_ & bits) != 0; }
	void ClearDirty(int bits) { dirtyFlag_ &= ~bits; }

	Sprite FindDirectChild(const std::string& name);
	const char* SpriteTypeToString(ObjectType type);

	glm::mat4 TRS(const glm::vec3& t, const glm::quat& r, const glm::vec3& s);

private:
	bool active_;

	std::string tag_;
	std::string name_;

	Mesh mesh_;
	Renderer renderer_;
	Animation animation_;

	std::weak_ptr<Sprite::element_type> parent_;

	std::vector<Sprite> children_;
	int dirtyFlag_;

	struct Transform {
		Transform() : scale(1) {}

		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		glm::vec3 eulerAngles;
	};

	Transform local_;
	Transform world_;

	glm::mat4 localToWorldMatrix_;
	glm::mat4 worldToLocalMatrix_;
};
