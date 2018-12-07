#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
#include "tools/math2.h"
#include "transforminternal.h"
#include "internal/async/guard.h"

ITransform::ITransform() : IComponent(MEMORY_NEW(TransformInternal)) {}
bool ITransform::IsAttachedToScene() { return _suede_dptr()->IsAttachedToScene(_shared_this()); }
void ITransform::AddChild(Transform child) { _suede_dptr()->AddChild(_shared_this(), child); }
void ITransform::RemoveChild(Transform child) { _suede_dptr()->RemoveChild(child); }
void ITransform::RemoveChildAt(uint index) { _suede_dptr()->RemoveChildAt(index); }
void ITransform::SetParent(Transform value) { _suede_dptr()->SetParent(_shared_this(), value); }
Transform ITransform::GetParent() const { return _suede_dptr()->GetParent(); }
glm::vec3 ITransform::TransformPoint(const glm::vec3& point) { return _suede_dptr()->TransformPoint(_shared_this(), point); }
glm::vec3 ITransform::TransformDirection(const glm::vec3& direction) { return _suede_dptr()->TransformDirection(_shared_this(), direction); }
glm::vec3 ITransform::InverseTransformPoint(const glm::vec3& point) { return _suede_dptr()->InverseTransformPoint(_shared_this(), point); }
glm::vec3 ITransform::InverseTransformDirection(const glm::vec3& direction) { return _suede_dptr()->InverseTransformDirection(_shared_this(), direction); }
Transform ITransform::FindChild(const std::string& path) { return _suede_dptr()->FindChild(path); }
int ITransform::GetChildCount() { return _suede_dptr()->GetChildCount(); }
Transform ITransform::GetChildAt(int i) { return _suede_dptr()->GetChildAt(i); }
ITransform::Enumerable ITransform::GetChildren() { return _suede_dptr()->GetChildren(); }
void ITransform::SetScale(const glm::vec3& value) { _suede_dptr()->SetScale(value); }
void ITransform::SetPosition(const glm::vec3& value) { _suede_dptr()->SetPosition(value); }
void ITransform::SetRotation(const glm::quat& value) { _suede_dptr()->SetRotation(value); }
void ITransform::SetEulerAngles(const glm::vec3& value) { _suede_dptr()->SetEulerAngles(value); }
glm::vec3 ITransform::GetScale() { return _suede_dptr()->GetScale(_shared_this()); }
glm::vec3 ITransform::GetPosition() { return _suede_dptr()->GetPosition(_shared_this()); }
glm::quat ITransform::GetRotation() { return _suede_dptr()->GetRotation(_shared_this()); }
glm::vec3 ITransform::GetEulerAngles() { return _suede_dptr()->GetEulerAngles(_shared_this()); }
void ITransform::SetLocalScale(const glm::vec3& value) { _suede_dptr()->SetLocalScale(value); }
void ITransform::SetLocalPosition(const glm::vec3& value) { _suede_dptr()->SetLocalPosition(value); }
void ITransform::SetLocalRotation(const glm::quat& value) { _suede_dptr()->SetLocalRotation(value); }
void ITransform::SetLocalEulerAngles(const glm::vec3& value) { _suede_dptr()->SetLocalEulerAngles(value); }
glm::vec3 ITransform::GetLocalScale() { return _suede_dptr()->GetLocalScale(_shared_this()); }
glm::vec3 ITransform::GetLocalPosition() { return _suede_dptr()->GetLocalPosition(_shared_this()); }
glm::quat ITransform::GetLocalRotation() { return _suede_dptr()->GetLocalRotation(_shared_this()); }
glm::vec3 ITransform::GetLocalEulerAngles() { return _suede_dptr()->GetLocalEulerAngles(_shared_this()); }
glm::mat4 ITransform::GetLocalToWorldMatrix() { return _suede_dptr()->GetLocalToWorldMatrix(_shared_this()); }
glm::mat4 ITransform::GetWorldToLocalMatrix() { return _suede_dptr()->GetWorldToLocalMatrix(_shared_this()); }
glm::vec3 ITransform::GetLocalToWorldPosition(const glm::vec3& position) { return _suede_dptr()->GetLocalToWorldPosition(_shared_this(), position); }
glm::vec3 ITransform::GetWorldToLocalPosition(const glm::vec3& position) { return _suede_dptr()->GetWorldToLocalPosition(_shared_this(), position); }
glm::vec3 ITransform::GetUp() { return _suede_dptr()->GetUp(_shared_this()); }
glm::vec3 ITransform::GetRight() { return _suede_dptr()->GetRight(_shared_this()); }
glm::vec3 ITransform::GetForward() { return _suede_dptr()->GetForward(_shared_this()); }

ZThread::Mutex TransformInternal::hierarchyMutex;

SUEDE_DEFINE_COMPONENT(ITransform, IComponent)

TransformInternal::TransformInternal() : ComponentInternal(ObjectType::Transform) {
	local_.scale = world_.scale = glm::vec3(1);
}

TransformInternal::~TransformInternal() {
}

bool TransformInternal::IsAttachedToScene(Transform self) {
	for (; self && self != World::GetRootTransform(); self = self->GetParent())
		;

	return !!self;
}

void TransformInternal::AddChild(Transform self, Transform child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		child->SetParent(self);
	}
}

void TransformInternal::RemoveChild(Transform child) {
	if (std::find(children_.begin(), children_.end(), child) != children_.end()) {
		child->SetParent(World::GetRootTransform());
	}
}

void TransformInternal::RemoveChildAt(uint index) {
	VERIFY_INDEX(index, children_.size(), NOARG);
	Transform child = children_[index];
	RemoveChild(child);
}

void TransformInternal::SetParent(Transform self, Transform value) {
	if (_suede_d_equals(value)) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	Transform oldParent = parent_.lock();
	if (oldParent != value) {
		ZTHREAD_LOCK_SCOPE(hierarchyMutex);
		ChangeParent(self, oldParent, value);
	}
}

glm::vec3 TransformInternal::TransformPoint(Transform self, const glm::vec3& point) {
	return glm::vec3(GetLocalToWorldMatrix(self) * glm::vec4(point, 1));
}

glm::vec3 TransformInternal::TransformDirection(Transform self, const glm::vec3& direction) {
	return glm::vec3(GetLocalToWorldMatrix(self) * glm::vec4(direction, 0));
}

glm::vec3 TransformInternal::InverseTransformPoint(Transform self, const glm::vec3& point) {
	return glm::vec3(GetWorldToLocalMatrix(self) * glm::vec4(point, 1));
}

glm::vec3 TransformInternal::InverseTransformDirection(Transform self, const glm::vec3& direction) {
	return glm::vec3(GetWorldToLocalMatrix(self) * glm::vec4(direction, 0));
}

Transform TransformInternal::FindChild(const std::string& path) {
	const char* back = path.c_str(), *fwd = back;

	TransformInternal* current = this;
	for (; (fwd = strchr(back, '/')) != nullptr; back = fwd + 1) {
		Transform child = current->FindDirectChild(std::string(back, fwd));
		if (!child) {
			return nullptr;
		}

		current = _suede_rptr(child);
	}

	return current->FindDirectChild(back);
}


void TransformInternal::SetScale(const glm::vec3& value) {
	ClearDirty(WorldScale);

	if (world_.scale != value) {
		world_.scale = value;
		SetDirty(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenScales();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(2, 0);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

void TransformInternal::SetPosition(const glm::vec3& value) {
	ClearDirty(WorldPosition);
	if (world_.position != value) {
		world_.position = value;
		SetDirty(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenPositions();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(0, 0);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

void TransformInternal::SetRotation(const glm::quat& value) {
	ClearDirty(WorldRotation);

	if (!Math::Approximately(glm::dot(world_.rotation, value), 0)) {
		world_.rotation = value;

		SetDirty(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(1, 0);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

void TransformInternal::SetEulerAngles(const glm::vec3& value) {
	ClearDirty(WorldEulerAngles);

	if (world_.eulerAngles != value) {
		world_.eulerAngles = value;

		SetDirty(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
		DirtyChildrenRotationsAndEulerAngles();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(1, 0);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

glm::vec3 TransformInternal::GetScale(Transform self) {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		glm::vec3 scale = GetLocalScale(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			scale *= self->GetScale();
		}

		world_.scale = scale;
		ClearDirty(WorldScale);
	}

	return world_.scale;
}

glm::vec3 TransformInternal::GetPosition(Transform self) {
	if (IsDirty(WorldPosition)) {
		if (IsDirty(LocalPosition)) {
			Debug::LogError("invalid state");
		}

		glm::vec3 position = GetLocalPosition(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			position = self->TransformPoint(position);
		}

		world_.position = position;
		ClearDirty(WorldPosition);
	}

	return world_.position;
}

glm::quat TransformInternal::GetRotation(Transform self) {
	if (!IsDirty(WorldRotation)) { return world_.rotation; }

	if (!IsDirty(WorldEulerAngles)) {
		world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
	}
	else {
		glm::quat localRotation;
		if (!IsDirty(LocalRotation)) {
			localRotation = GetLocalRotation(self);
		}
		else {
			if (IsDirty(LocalEulerAngles)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
			ClearDirty(LocalRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			localRotation = self->GetRotation() * localRotation;
		}

		world_.rotation = localRotation;
	}

	ClearDirty(WorldRotation);

	return world_.rotation;
}

glm::vec3 TransformInternal::GetEulerAngles(Transform self) {
	if (!IsDirty(WorldEulerAngles)) { return world_.eulerAngles; }

	glm::quat worldRotation;
	if (!IsDirty(WorldRotation)) {
		worldRotation = GetRotation(self);
	}
	else {
		glm::quat localRotation;

		if (!IsDirty(LocalRotation)) {
			localRotation = GetLocalRotation(self);
		}
		else {
			if (IsDirty(LocalEulerAngles)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
			ClearDirty(LocalRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			localRotation = self->GetRotation() * localRotation;
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
		SetDirty(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenScales();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(2, 1);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

void TransformInternal::SetLocalPosition(const glm::vec3& value) {
	ClearDirty(LocalPosition);
	if (local_.position != value) {
		local_.position = value;
		SetDirty(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
		DirtyChildrenPositions();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(0, 1);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

void TransformInternal::SetLocalRotation(const glm::quat& value) {
	ClearDirty(LocalRotation);
	if (!Math::Approximately(glm::dot(local_.rotation, value), 0)) {
		local_.rotation = value;
		SetDirty(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(1, 1);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

void TransformInternal::SetLocalEulerAngles(const glm::vec3& value) {
	ClearDirty(LocalEulerAngles);
	if (local_.eulerAngles != value) {
		local_.eulerAngles = value;
		SetDirty(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		gameObject_.lock()->RecalculateBounds();

		GameObjectTransformChangedEventPtr e = NewWorldEvent<GameObjectTransformChangedEventPtr>();
		e->prs = Math::MakeDword(1, 1);
		e->go = gameObject_.lock();
		World::FireEvent(e);
	}
}

glm::vec3 TransformInternal::GetLocalScale(Transform self) {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		glm::vec3 scale = GetScale(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			scale /= self->GetScale();
		}

		local_.scale = scale;
		ClearDirty(LocalScale);
	}

	return local_.scale;
}

glm::vec3 TransformInternal::GetLocalPosition(Transform self) {
	if (IsDirty(LocalPosition)) {
		if (IsDirty(WorldPosition)) {
			Debug::LogError("invalid state");
		}

		glm::vec3 position = GetPosition(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			position = self->InverseTransformPoint(position);
		}

		local_.position = position;
		ClearDirty(LocalPosition);
	}

	return local_.position;
}

glm::quat TransformInternal::GetLocalRotation(Transform self) {
	if (!IsDirty(LocalRotation)) { return local_.rotation; }

	if (!IsDirty(LocalEulerAngles)) {
		local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
	}
	else {
		glm::quat worldRotation;
		if (!IsDirty(WorldRotation)) {
			worldRotation = GetRotation(self);
		}
		else {
			if (IsDirty(WorldEulerAngles)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
			ClearDirty(WorldRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			worldRotation = glm::inverse(self->GetRotation()) * worldRotation;
		}

		local_.rotation = worldRotation;
	}

	ClearDirty(LocalRotation);

	return local_.rotation;
}

glm::vec3 TransformInternal::GetLocalEulerAngles(Transform self) {
	if (!IsDirty(LocalEulerAngles)) { return local_.eulerAngles; }

	glm::quat localRotation;
	if (!IsDirty(LocalRotation)) {
		localRotation = GetLocalRotation(self);
	}
	else {
		glm::quat worldRotation;

		if (!IsDirty(WorldRotation)) {
			worldRotation = GetRotation(self);
		}
		else {
			if (IsDirty(WorldEulerAngles)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
			ClearDirty(WorldRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			worldRotation = glm::inverse(self->GetRotation()) * worldRotation;
		}

		localRotation = local_.rotation = worldRotation;
		ClearDirty(LocalRotation);
	}

	glm::vec3 angles = glm::eulerAngles(localRotation);
	local_.eulerAngles = Math::Degrees(angles);

	ClearDirty(LocalEulerAngles);

	return local_.eulerAngles;
}

glm::mat4 TransformInternal::GetLocalToWorldMatrix(Transform self) {
	if (IsDirty(LocalToWorldMatrix)) {
		glm::mat4 matrix = Math::TRS(GetLocalPosition(self), GetLocalRotation(self), GetLocalScale(self));
		if (!IsNullOrRoot(self = self->GetParent())) {
			matrix = self->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		ClearDirty(LocalToWorldMatrix);
	}

	return localToWorldMatrix_;
}

glm::mat4 TransformInternal::GetWorldToLocalMatrix(Transform self) {
	if (IsDirty(WorldToLocalMatrix)) {
		worldToLocalMatrix_ = glm::inverse(GetLocalToWorldMatrix(self));
		ClearDirty(WorldToLocalMatrix);
	}

	return worldToLocalMatrix_;
}

glm::vec3 TransformInternal::GetLocalToWorldPosition(Transform self, const glm::vec3& position) {
	return glm::vec3(GetLocalToWorldMatrix(self) * glm::vec4(position, 1));
}

glm::vec3 TransformInternal::GetWorldToLocalPosition(Transform self, const glm::vec3& position) {
	return glm::vec3(GetWorldToLocalMatrix(self) * glm::vec4(position, 1));
}

glm::vec3 TransformInternal::GetUp(Transform self) {
	return GetRotation(self) * glm::vec3(0, 1, 0);
}

glm::vec3 TransformInternal::GetRight(Transform self) {
	return GetRotation(self) * glm::vec3(1, 0, 0);
}

glm::vec3 TransformInternal::GetForward(Transform self) {
	return GetRotation(self) * glm::vec3(0, 0, -1);
}

void TransformInternal::SetDirty(int bits) {
	DirtyBits::SetDirty(bits);

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
	for(Transform transform : GetChildren()) {
		transform->GetLocalScale();

		TransformInternal* child = _suede_rptr(transform);
		child->SetDirty(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
		child->DirtyChildrenScales();
	}
}

void TransformInternal::DirtyChildrenPositions() {
	for (Transform transform : GetChildren()) {
		transform->GetLocalPosition();

		TransformInternal* child = _suede_rptr(transform);
		child->SetDirty(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
		child->DirtyChildrenPositions();
	}
}

void TransformInternal::DirtyChildrenRotationsAndEulerAngles() {
	for (Transform transform : GetChildren()) {
		transform->GetLocalRotation();
		transform->GetLocalEulerAngles();
		TransformInternal* child = _suede_rptr(transform);
		child->SetDirty(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
		child->DirtyChildrenRotationsAndEulerAngles();
	}
}

bool TransformInternal::IsNullOrRoot(Transform transform) {
	return !transform || transform == World::GetRootTransform();
}

Transform TransformInternal::FindDirectChild(const std::string& name) {
	for (int i = 0; i < children_.size(); ++i) {
		if (name == children_[i]->GetGameObject()->GetName()) {
			return children_[i];
		}
	}

	return nullptr;
}

void TransformInternal::ChangeParent(Transform self, Transform oldParent, Transform newParent) {
	if (oldParent) { // remove from old parent.
		TransformInternal* optr = _suede_rptr(oldParent);
		RemoveChildItem(optr->children_, self);
	}

	if (newParent) {
		TransformInternal* nptr = _suede_rptr(newParent);
		AddChildItem(nptr->children_, self);
	}

	if (oldParent) {
		oldParent->GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
	}

	if (newParent) {
		newParent->GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
	}

	parent_ = newParent;

	// Clear dirty flags.
	GetScale(self);
	GetRotation(self);
	GetPosition(self);
	SetDirty(LocalScale | LocalRotation | LocalPosition | LocalEulerAngles);

	if (IsAttachedToScene(self)) {
		GameObjectParentChangedEventPtr e = NewWorldEvent<GameObjectParentChangedEventPtr>();
		e->go = self->GetGameObject();
		World::FireEvent(e);
	}
}

bool TransformInternal::AddChildItem(Children & children, Transform child) {
	if (std::find(children.begin(), children.end(), child) == children.end()) {
		children.push_back(child);
		return true;
	}

	return false;
}

bool TransformInternal::RemoveChildItem(Children& children, Transform child) {
	Children::iterator pos = std::find(children.begin(), children.end(), child);
	if (pos != children.end()) {
		children.erase(pos);
		return true;
	}

	return false;
}
