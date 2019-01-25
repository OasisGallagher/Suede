#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
#include "tools/math2.h"
#include "transforminternal.h"
#include "internal/async/async.h"

ITransform::ITransform() : IComponent(MEMORY_NEW(TransformInternal, this)) {}
bool ITransform::IsAttachedToScene() { return _suede_dptr()->IsAttachedToScene(); }
void ITransform::AddChild(Transform child) { _suede_dptr()->AddChild(child); }
void ITransform::RemoveChild(Transform child) { _suede_dptr()->RemoveChild(child); }
void ITransform::RemoveChildAt(uint index) { _suede_dptr()->RemoveChildAt(index); }
void ITransform::SetParent(Transform value) { _suede_dptr()->SetParent(value); }
Transform ITransform::GetParent() const { return _suede_dptr()->GetParent(); }
glm::vec3 ITransform::TransformPoint(const glm::vec3& point) { return _suede_dptr()->TransformPoint(point); }
glm::vec3 ITransform::TransformDirection(const glm::vec3& direction) { return _suede_dptr()->TransformDirection(direction); }
glm::vec3 ITransform::InverseTransformPoint(const glm::vec3& point) { return _suede_dptr()->InverseTransformPoint(point); }
glm::vec3 ITransform::InverseTransformDirection(const glm::vec3& direction) { return _suede_dptr()->InverseTransformDirection(direction); }
Transform ITransform::FindChild(const std::string& path) { return _suede_dptr()->FindChild(path); }
int ITransform::GetChildCount() { return _suede_dptr()->GetChildCount(); }
Transform ITransform::GetChildAt(int i) { return _suede_dptr()->GetChildAt(i); }
ITransform::Enumerable ITransform::GetChildren() { return _suede_dptr()->GetChildren(); }
void ITransform::SetScale(const glm::vec3& value) { _suede_dptr()->SetScale(value); }
void ITransform::SetPosition(const glm::vec3& value) { _suede_dptr()->SetPosition(value); }
void ITransform::SetRotation(const glm::quat& value) { _suede_dptr()->SetRotation(value); }
void ITransform::SetEulerAngles(const glm::vec3& value) { _suede_dptr()->SetEulerAngles(value); }
glm::vec3 ITransform::GetScale() { return _suede_dptr()->GetScale(); }
glm::vec3 ITransform::GetPosition() { return _suede_dptr()->GetPosition(); }
glm::quat ITransform::GetRotation() { return _suede_dptr()->GetRotation(); }
glm::vec3 ITransform::GetEulerAngles() { return _suede_dptr()->GetEulerAngles(); }
void ITransform::SetLocalScale(const glm::vec3& value) { _suede_dptr()->SetLocalScale(value); }
void ITransform::SetLocalPosition(const glm::vec3& value) { _suede_dptr()->SetLocalPosition(value); }
void ITransform::SetLocalRotation(const glm::quat& value) { _suede_dptr()->SetLocalRotation(value); }
void ITransform::SetLocalEulerAngles(const glm::vec3& value) { _suede_dptr()->SetLocalEulerAngles(value); }
glm::vec3 ITransform::GetLocalScale() { return _suede_dptr()->GetLocalScale(); }
glm::vec3 ITransform::GetLocalPosition() { return _suede_dptr()->GetLocalPosition(); }
glm::quat ITransform::GetLocalRotation() { return _suede_dptr()->GetLocalRotation(); }
glm::vec3 ITransform::GetLocalEulerAngles() { return _suede_dptr()->GetLocalEulerAngles(); }
glm::mat4 ITransform::GetLocalToWorldMatrix() { return _suede_dptr()->GetLocalToWorldMatrix(); }
glm::mat4 ITransform::GetWorldToLocalMatrix() { return _suede_dptr()->GetWorldToLocalMatrix(); }
glm::vec3 ITransform::GetLocalToWorldPosition(const glm::vec3& position) { return _suede_dptr()->GetLocalToWorldPosition(position); }
glm::vec3 ITransform::GetWorldToLocalPosition(const glm::vec3& position) { return _suede_dptr()->GetWorldToLocalPosition(position); }
glm::vec3 ITransform::GetUp() { return _suede_dptr()->GetUp(); }
glm::vec3 ITransform::GetRight() { return _suede_dptr()->GetRight(); }
glm::vec3 ITransform::GetForward() { return _suede_dptr()->GetForward(); }

ZThread::Mutex TransformInternal::hierarchyMutex;

SUEDE_DEFINE_COMPONENT_INTERNAL(Transform, Component)

TransformInternal::TransformInternal(ITransform* self) : ComponentInternal(self, ObjectType::Transform), parent_(nullptr) {
	local_.scale = world_.scale = glm::vec3(1);
}

TransformInternal::~TransformInternal() {
}

bool TransformInternal::IsAttachedToScene() {
	ITransform* self = _suede_self();
	for (; self && self != World::GetRootTransform(); self = self->GetParent().get())
		;

	return !!self;
}

void TransformInternal::AddChild(Transform child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		child->SetParent(_suede_self());
	}
}

void TransformInternal::RemoveChild(Transform child) {
	if (std::find(children_.begin(), children_.end(), child) != children_.end()) {
		child->SetParent(World::GetRootTransform());
	}
}

void TransformInternal::RemoveChildAt(uint index) {
	SUEDE_VERIFY_INDEX(index, children_.size(), SUEDE_NOARG);
	Transform child = children_[index];
	RemoveChild(child);
}

void TransformInternal::SetParent(Transform value) {
	if (value && value == self_) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	ITransform* oldParent = parent_;
	if (oldParent != value) {
		ZTHREAD_LOCK_SCOPE(hierarchyMutex);
		ChangeParent(oldParent, value);
	}
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

		current = _suede_rptr(child);
	}

	return current->FindDirectChild(back);
}


void TransformInternal::SetScale(const glm::vec3& value) {
	ClearDirty(WorldScale);

	if (!Math::Approximately(world_.scale, value)) {
		DirtyChildrenScales();

		world_.scale = value;
		SetDirty(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(2, 0)));
	}
}

void TransformInternal::SetPosition(const glm::vec3& value) {
	ClearDirty(WorldPosition);
	if (!Math::Approximately(world_.position, value)) {
		DirtyChildrenPositions();

		world_.position = value;
		SetDirty(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(0, 0)));
	}
}

void TransformInternal::SetRotation(const glm::quat& value) {
	ClearDirty(WorldRotation);

	if (!Math::Approximately(world_.rotation, value)) {
		DirtyChildrenRotationsAndEulerAngles();

		world_.rotation = value;

		SetDirty(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(1, 0)));
	}
}

void TransformInternal::SetEulerAngles(const glm::vec3& value) {
	ClearDirty(WorldEulerAngles);

	if (!Math::Approximately(world_.eulerAngles, value)) {
		DirtyChildrenRotationsAndEulerAngles();

		world_.eulerAngles = value;

		SetDirty(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(1, 0)));
	}
}

glm::vec3 TransformInternal::GetScale() {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		ITransform* self = _suede_self();
		glm::vec3 scale = GetLocalScale();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			scale *= self->GetScale();
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

		ITransform* self = _suede_self();
		glm::vec3 position = GetLocalPosition();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			position = self->TransformPoint(position);
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

		ITransform* self = _suede_self();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			localRotation = self->GetRotation() * localRotation;
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

		ITransform* self = _suede_self();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
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
	if (!Math::Approximately(local_.scale, value)) {
		DirtyChildrenScales();

		local_.scale = value;
		SetDirty(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);

		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(2, 1)));
	}
}

void TransformInternal::SetLocalPosition(const glm::vec3& value) {
	ClearDirty(LocalPosition);
	if (!Math::Approximately(local_.position, value)) {
		DirtyChildrenPositions();

		local_.position = value;

		SetDirty(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(0, 1)));
	}
}

void TransformInternal::SetLocalRotation(const glm::quat& value) {
	ClearDirty(LocalRotation);
	if (!Math::Approximately(glm::dot(local_.rotation, value), 0)) {
		DirtyChildrenRotationsAndEulerAngles();

		local_.rotation = value;
		SetDirty(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(1, 1)));
	}
}

void TransformInternal::SetLocalEulerAngles(const glm::vec3& value) {
	ClearDirty(LocalEulerAngles);
	if (!Math::Approximately(local_.eulerAngles, value)) {
		DirtyChildrenRotationsAndEulerAngles();

		local_.eulerAngles = value;
		SetDirty(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);

		gameObject_->RecalculateBounds();

		World::FireEvent(new GameObjectTransformChangedEvent(gameObject_, Math::MakeDword(1, 1)));
	}
}

glm::vec3 TransformInternal::GetLocalScale() {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		ITransform* self = _suede_self();
		glm::vec3 scale = GetScale();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			scale /= self->GetScale();
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

		ITransform* self = _suede_self();
		glm::vec3 position = GetPosition();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			position = self->InverseTransformPoint(position);
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

		ITransform* self = _suede_self();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			worldRotation = glm::inverse(self->GetRotation()) * worldRotation;
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

		ITransform* self = _suede_self();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
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

glm::mat4 TransformInternal::GetLocalToWorldMatrix() {
	if (IsDirty(LocalToWorldMatrix)) {
		glm::mat4 matrix = glm::trs(GetLocalPosition(), GetLocalRotation(), GetLocalScale());

		ITransform* self = _suede_self();
		if (!IsNullOrRoot(self = self->GetParent().get())) {
			matrix = self->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		ClearDirty(LocalToWorldMatrix);

		GetGameObject()->SendMessage(GameObjectMessageLocalToWorldMatrixModified, nullptr);
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
	return GetRotation() * glm::vec3(0, 1, 0);
}

glm::vec3 TransformInternal::GetRight() {
	return GetRotation() * glm::vec3(1, 0, 0);
}

glm::vec3 TransformInternal::GetForward() {
	return GetRotation() * glm::vec3(0, 0, -1);
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

void TransformInternal::ChangeParent(Transform oldParent, Transform newParent) {
	if (oldParent) { // remove from old parent.
		TransformInternal* optr = _suede_rptr(oldParent);
		RemoveChildItem(optr->children_, _suede_self());
	}

	if (newParent) {
		TransformInternal* nptr = _suede_rptr(newParent);
		AddChildItem(nptr->children_, _suede_self());
	}

	if (oldParent) {
		oldParent->GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
	}

	if (newParent) {
		newParent->GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
	}

	parent_ = newParent.get();

	// Clear dirty flags.
	GetScale();
	GetRotation();
	GetPosition();
	SetDirty(LocalScale | LocalRotation | LocalPosition | LocalEulerAngles);

	if (IsAttachedToScene()) {
		World::FireEvent(new GameObjectParentChangedEvent(GetGameObject()));
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
