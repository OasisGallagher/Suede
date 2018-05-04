#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
#include "tools/math2.h"
#include "transforminternal.h"

TransformInternal::TransformInternal() : ObjectInternal(ObjectTypeTransform), dirtyBits_(0) {
	local_.scale = world_.scale = glm::vec3(1);
}

void TransformInternal::SetEntity(Entity value) {
	if (entity_.lock() != value) {
		entity_ = value;
		value->SetTransform(suede_dynamic_cast<Transform>(shared_from_this()));
	}
}

void TransformInternal::AddChild(Transform child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		children_.push_back(child);
		child->SetParent(suede_dynamic_cast<Transform>(shared_from_this()));
	}
}

void TransformInternal::RemoveChild(Transform child) {
	std::vector<Transform>::iterator pos = std::find(children_.begin(), children_.end(), child);
	if (pos != children_.end()) {
		child->SetParent(WorldInstance()->GetRootTransform());
		children_.erase(pos);
	}
}

void TransformInternal::RemoveChildAt(uint index) {
	if (index >= children_.size()) {
		Debug::LogError("index out of range.");
		return;
	}

	Transform child = children_[index];
	child->SetParent(WorldInstance()->GetRootTransform());
	children_.erase(children_.begin() + index);
}

void TransformInternal::SetParent(Transform value) {
	if (value.get() == this) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	Transform oldParent = parent_.lock();
	if (oldParent == value) {
		return;
	}

	Transform thisSp = suede_dynamic_cast<Transform>(shared_from_this());
	if (oldParent) {
		oldParent->RemoveChild(thisSp);
	}

	if (value) {
		value->AddChild(thisSp);
	}

	parent_ = value;

	// Clear dirty flags.
	GetScale();
	GetRotation();
	GetPosition();

	SetDiry(LocalScale | LocalRotation | LocalPosition | LocalEulerAngles);

	EntityParentChangedEventPointer e = NewWorldEvent<EntityParentChangedEventPointer>();
	e->entity = thisSp->GetEntity();
	WorldInstance()->FireEvent(e);
}

glm::vec3 TransformInternal::TransformPoint(const glm::vec3& point) {
	return glm::vec3(GetLocalToWorldMatrix() * glm::vec4(point, 1));
}

glm::vec3 TransformInternal::TransformDirection(const glm::vec3& direction) {
	return glm::vec3(GetLocalToWorldMatrix() * glm::vec4(direction, 0));
}

glm::vec3 TransformInternal::InverseTransformPoint(const glm::vec3& point) {
	return glm::vec3(GetWorldToLocalMatrix() * glm::vec4(point, 1));
}

glm::vec3 TransformInternal::InverseTransformDirection(const glm::vec3& direction) {
	return glm::vec3(GetWorldToLocalMatrix() * glm::vec4(direction, 0));
}

Transform TransformInternal::FindChild(const std::string& path) {
	const char* back = path.c_str(), *fwd = back;

	TransformInternal* current = this;
	for (; (fwd = strchr(back, '/')) != nullptr; back = fwd + 1) {
		Transform child = current->FindDirectChild(std::string(back, fwd));
		if (!child) {
			return nullptr;
		}

		current = dynamic_cast<TransformInternal*>(child.get());
	}

	return current->FindDirectChild(back);
}


void TransformInternal::SetScale(const glm::vec3& value) {
	ClearDirty(WorldScale);

	if (world_.scale != value) {
		world_.scale = value;
		SetDiry(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenScales();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(2, 0);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

void TransformInternal::SetPosition(const glm::vec3& value) {
	ClearDirty(WorldPosition);
	if (world_.position != value) {
		world_.position = value;
		SetDiry(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenPositions();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(0, 0);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

void TransformInternal::SetRotation(const glm::quat& value) {
	ClearDirty(WorldRotation);

	if (!Math::Approximately(glm::dot(world_.rotation, value), 0)) {
		world_.rotation = value;

		SetDiry(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(1, 0);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

void TransformInternal::SetEulerAngles(const glm::vec3& value) {
	ClearDirty(WorldEulerAngles);

	if (world_.eulerAngles != value) {
		world_.eulerAngles = value;

		SetDiry(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
		DirtyChildrenRotationsAndEulerAngles();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(1, 0);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

glm::vec3 TransformInternal::GetScale() {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::vec3 scale = GetLocalScale();
		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			scale *= current->GetScale();
		}

		world_.scale = scale;
		ClearDirty(WorldScale);
	}

	return world_.scale;
}

glm::vec3 TransformInternal::GetPosition() {
	if (IsDirty(WorldPosition)) {
		if (IsDirty(LocalPosition)) {
			Debug::LogError("invalid state");
		}

		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::vec3 position = GetLocalPosition();
		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			position = current->TransformPoint(position);
		}

		world_.position = position;
		ClearDirty(WorldPosition);
	}

	return world_.position;
}

glm::quat TransformInternal::GetRotation() {
	if (!IsDirty(WorldRotation)) { return world_.rotation; }

	if (!IsDirty(WorldEulerAngles)) {
		world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
	}
	else {
		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::quat localRotation;
		if (!IsDirty(LocalRotation)) {
			localRotation = GetLocalRotation();
		}
		else {
			if (IsDirty(LocalEulerAngles)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
			ClearDirty(LocalRotation);
		}

		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			localRotation = current->GetRotation() * localRotation;
		}

		world_.rotation = localRotation;
	}

	ClearDirty(WorldRotation);

	return world_.rotation;
}

glm::vec3 TransformInternal::GetEulerAngles() {
	if (!IsDirty(WorldEulerAngles)) { return world_.eulerAngles; }

	glm::quat worldRotation;
	if (!IsDirty(WorldRotation)) {
		worldRotation = GetRotation();
	}
	else {
		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::quat localRotation;

		if (!IsDirty(LocalRotation)) {
			localRotation = GetLocalRotation();
		}
		else {
			if (IsDirty(LocalEulerAngles)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
			ClearDirty(LocalRotation);
		}

		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			localRotation = current->GetRotation() * localRotation;
		}

		worldRotation = world_.rotation = localRotation;
		ClearDirty(WorldRotation);
	}

	world_.eulerAngles = Math::Degrees(glm::eulerAngles(worldRotation));

	ClearDirty(WorldEulerAngles);

	return world_.eulerAngles;
}

void TransformInternal::SetLocalScale(const glm::vec3& value) {
	ClearDirty(LocalScale);
	if (local_.scale != value) {
		local_.scale = value;
		SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenScales();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(2, 1);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

void TransformInternal::SetLocalPosition(const glm::vec3& value) {
	ClearDirty(LocalPosition);
	if (local_.position != value) {
		local_.position = value;
		SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
		DirtyChildrenPositions();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(0, 1);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

void TransformInternal::SetLocalRotation(const glm::quat& value) {
	ClearDirty(LocalRotation);
	if (!Math::Approximately(glm::dot(local_.rotation, value), 0)) {
		local_.rotation = value;
		SetDiry(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(1, 1);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

void TransformInternal::SetLocalEulerAngles(const glm::vec3& value) {
	ClearDirty(LocalEulerAngles);
	if (local_.eulerAngles != value) {
		local_.eulerAngles = value;
		SetDiry(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		entity_.lock()->RecalculateBounds();

		EntityTransformChangedEventPointer e = NewWorldEvent<EntityTransformChangedEventPointer>();
		e->prs = Math::MakeDword(1, 1);
		e->entity = entity_.lock();
		WorldInstance()->FireEvent(e);
	}
}

glm::vec3 TransformInternal::GetLocalScale() {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::vec3 scale = GetScale();
		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			scale /= current->GetScale();
		}

		local_.scale = scale;
		ClearDirty(LocalScale);
	}

	return local_.scale;
}

glm::vec3 TransformInternal::GetLocalPosition() {
	if (IsDirty(LocalPosition)) {
		if (IsDirty(WorldPosition)) {
			Debug::LogError("invalid state");
		}

		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::vec3 position = GetPosition();
		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			position = current->InverseTransformPoint(position);
		}

		local_.position = position;
		ClearDirty(LocalPosition);
	}

	return local_.position;
}

glm::quat TransformInternal::GetLocalRotation() {
	if (!IsDirty(LocalRotation)) { return local_.rotation; }

	if (!IsDirty(LocalEulerAngles)) {
		local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
	}
	else {
		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::quat worldRotation;
		if (!IsDirty(WorldRotation)) {
			worldRotation = GetRotation();
		}
		else {
			if (IsDirty(WorldEulerAngles)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
			ClearDirty(WorldRotation);
		}

		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			worldRotation = glm::inverse(current->GetRotation()) * worldRotation;
		}

		local_.rotation = worldRotation;
	}

	ClearDirty(LocalRotation);

	return local_.rotation;
}

glm::vec3 TransformInternal::GetLocalEulerAngles() {
	if (!IsDirty(LocalEulerAngles)) { return local_.eulerAngles; }

	glm::quat localRotation;
	if (!IsDirty(LocalRotation)) {
		localRotation = GetLocalRotation();
	}
	else {
		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::quat worldRotation;

		if (!IsDirty(WorldRotation)) {
			worldRotation = GetRotation();
		}
		else {
			if (IsDirty(WorldEulerAngles)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
			ClearDirty(WorldRotation);
		}

		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			worldRotation = glm::inverse(current->GetRotation()) * worldRotation;
		}

		localRotation = local_.rotation = worldRotation;
		ClearDirty(LocalRotation);
	}

	glm::vec3 angles = glm::eulerAngles(localRotation);
	local_.eulerAngles = Math::Degrees(angles);

	ClearDirty(LocalEulerAngles);

	return local_.eulerAngles;
}

glm::mat4 TransformInternal::GetLocalToWorldMatrix() {
	if (IsDirty(LocalToWorldMatrix)) {
		Transform current = suede_dynamic_cast<Transform>(shared_from_this());
		glm::mat4 matrix = Math::TRS(GetLocalPosition(), GetLocalRotation(), GetLocalScale());
		if ((current = current->GetParent()) != WorldInstance()->GetRootTransform()) {
			matrix = current->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		ClearDirty(LocalToWorldMatrix);
	}

	return localToWorldMatrix_;
}

glm::mat4 TransformInternal::GetWorldToLocalMatrix() {
	if (IsDirty(WorldToLocalMatrix)) {
		worldToLocalMatrix_ = glm::inverse(GetLocalToWorldMatrix());
		ClearDirty(WorldToLocalMatrix);
	}

	return worldToLocalMatrix_;
}

glm::vec3 TransformInternal::GetLocalToWorldPosition(const glm::vec3& position) {
	return glm::vec3(GetLocalToWorldMatrix() * glm::vec4(position, 1));
}

glm::vec3 TransformInternal::GetWorldToLocalPosition(const glm::vec3& position) {
	return glm::vec3(GetWorldToLocalMatrix() * glm::vec4(position, 1));
}

glm::vec3 TransformInternal::GetUp() {
	return world_.rotation * glm::vec3(0, 1, 0);
}

glm::vec3 TransformInternal::GetRight() {
	return world_.rotation * glm::vec3(1, 0, 0);
}

glm::vec3 TransformInternal::GetForward() {
	return world_.rotation * glm::vec3(0, 0, -1);
}

void TransformInternal::SetDiry(int bits) {
	dirtyBits_ |= bits;
	if (IsDirty(LocalScale) && IsDirty(WorldScale)) {
		Debug::LogError("invalid state");
	}

	if (IsDirty(LocalPosition) && IsDirty(WorldPosition)) {
		Debug::LogError("invalid state");
	}

	if (IsDirty(LocalRotation) && IsDirty(WorldRotation) && IsDirty(LocalEulerAngles) && IsDirty(WorldEulerAngles)) {
		Debug::LogError("invalid state");
	}
}

void TransformInternal::DirtyChildrenScales() {
	for (int i = 0; i < GetChildCount(); ++i) {
		TransformInternal* child = dynamic_cast<TransformInternal*>(GetChildAt(i).get());
		child->GetLocalScale();
		child->SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void TransformInternal::DirtyChildrenPositions() {
	for (int i = 0; i < GetChildCount(); ++i) {
		TransformInternal* child = dynamic_cast<TransformInternal*>(GetChildAt(i).get());
		child->GetLocalPosition();
		child->SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void TransformInternal::DirtyChildrenRotationsAndEulerAngles() {
	for (int i = 0; i < GetChildCount(); ++i) {
		TransformInternal* child = dynamic_cast<TransformInternal*>(GetChildAt(i).get());
		child->GetLocalRotation();
		child->GetLocalEulerAngles();
		child->SetDiry(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

Transform TransformInternal::FindDirectChild(const std::string& name) {
	for (int i = 0; i < children_.size(); ++i) {
		if (name == children_[i]->GetEntity()->GetName()) {
			return children_[i];
		}
	}

	return nullptr;
}
