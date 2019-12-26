#include "scene.h"
#include "math/mathf.h"
#include "transforminternal.h"

Transform::Transform() : Component(new TransformInternal) {}
bool Transform::IsAttachedToScene() { return _suede_dptr()->IsAttachedToScene(this); }
void Transform::AddChild(Transform* child) { _suede_dptr()->AddChild(this, child); }
void Transform::RemoveChild(Transform* child) { _suede_dptr()->RemoveChild(child); }
void Transform::RemoveChildAt(uint index) { _suede_dptr()->RemoveChildAt(index); }
void Transform::SetParent(Transform* value) { _suede_dptr()->SetParent(this, value); }
Transform* Transform::GetParent() const { return _suede_dptr()->GetParent(); }
Vector3 Transform::TransformPoint(const Vector3& point) { return _suede_dptr()->TransformPoint(this, point); }
Vector3 Transform::TransformDirection(const Vector3& direction) { return _suede_dptr()->TransformDirection(this, direction); }
Vector3 Transform::InverseTransformPoint(const Vector3& point) { return _suede_dptr()->InverseTransformPoint(this, point); }
Vector3 Transform::InverseTransformDirection(const Vector3& direction) { return _suede_dptr()->InverseTransformDirection(this, direction); }
Transform* Transform::FindChild(const std::string& path) { return _suede_dptr()->FindChild(path); }
int Transform::GetChildCount() { return _suede_dptr()->GetChildCount(); }
Transform* Transform::GetChildAt(int i) { return _suede_dptr()->GetChildAt(i); }
void Transform::SetScale(const Vector3& value) { _suede_dptr()->SetScale(value); }
void Transform::SetPosition(const Vector3& value) { _suede_dptr()->SetPosition(value); }
void Transform::SetRotation(const Quaternion& value) { _suede_dptr()->SetRotation(value); }
void Transform::SetEulerAngles(const Vector3& value) { _suede_dptr()->SetEulerAngles(value); }
Vector3 Transform::GetScale() { return _suede_dptr()->GetScale(this); }
Vector3 Transform::GetPosition() { return _suede_dptr()->GetPosition(this); }
Quaternion Transform::GetRotation() { return _suede_dptr()->GetRotation(this); }
Vector3 Transform::GetEulerAngles() { return _suede_dptr()->GetEulerAngles(this); }
void Transform::SetLocalScale(const Vector3& value) { _suede_dptr()->SetLocalScale(value); }
void Transform::SetLocalPosition(const Vector3& value) { _suede_dptr()->SetLocalPosition(value); }
void Transform::SetLocalRotation(const Quaternion& value) { _suede_dptr()->SetLocalRotation(value); }
void Transform::SetLocalEulerAngles(const Vector3& value) { _suede_dptr()->SetLocalEulerAngles(value); }
Vector3 Transform::GetLocalScale() { return _suede_dptr()->GetLocalScale(this); }
Vector3 Transform::GetLocalPosition() { return _suede_dptr()->GetLocalPosition(this); }
Quaternion Transform::GetLocalRotation() { return _suede_dptr()->GetLocalRotation(this); }
Vector3 Transform::GetLocalEulerAngles() { return _suede_dptr()->GetLocalEulerAngles(this); }
Matrix4 Transform::GetLocalToWorldMatrix() { return _suede_dptr()->GetLocalToWorldMatrix(this); }
Matrix4 Transform::GetWorldToLocalMatrix() { return _suede_dptr()->GetWorldToLocalMatrix(this); }
Vector3 Transform::GetLocalToWorldPosition(const Vector3& position) { return _suede_dptr()->GetLocalToWorldPosition(this, position); }
Vector3 Transform::GetWorldToLocalPosition(const Vector3& position) { return _suede_dptr()->GetWorldToLocalPosition(this, position); }
Vector3 Transform::GetUp() { return _suede_dptr()->GetUp(this); }
Vector3 Transform::GetRight() { return _suede_dptr()->GetRight(this); }
Vector3 Transform::GetForward() { return _suede_dptr()->GetForward(this); }

std::mutex TransformInternal::hierarchyMutex;

SUEDE_DEFINE_COMPONENT_INTERNAL(Transform, Component)

TransformInternal::TransformInternal() : ComponentInternal(ObjectType::Transform), parent_(nullptr) {
	local_.scale = world_.scale = Vector3(1);
}

TransformInternal::~TransformInternal() {
}

bool TransformInternal::IsAttachedToScene(Transform* self) {
	for (; self && self != gameObject_->GetScene()->GetRootTransform(); self = self->GetParent())
		;

	return !!self;
}

void TransformInternal::AddChild(Transform* self, Transform* child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		child->SetParent(self);
	}
}

void TransformInternal::RemoveChild(Transform* child) {
	if (std::find(children_.begin(), children_.end(), child) != children_.end()) {
		child->SetParent(gameObject_->GetScene()->GetRootTransform());
	}
}

void TransformInternal::RemoveChildAt(uint index) {
	SUEDE_VERIFY_INDEX(index, children_.size(), SUEDE_NOARG);
	Transform* child = children_[index].get();
	RemoveChild(child);
}

void TransformInternal::SetParent(Transform* self, Transform* value) {
	if (self == value) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	Transform* oldParent = parent_;
	if (oldParent != value) {
		std::lock_guard<std::mutex> lock(hierarchyMutex);
		ChangeParent(self, oldParent, value);
	}
}

Vector3 TransformInternal::TransformPoint(Transform* self, const Vector3& point) {
	Vector4 p = GetLocalToWorldMatrix(self) * Vector4(point.x, point.y, point.z, 1);
	return Vector3(p.x, p.y, p.z);
}

Vector3 TransformInternal::TransformDirection(Transform* self, const Vector3& direction) {
	Vector4 d = GetLocalToWorldMatrix(self) * Vector4(direction.x, direction.y, direction.z, 0);
	return Vector3(d.x, d.y, d.z);
}

Vector3 TransformInternal::InverseTransformPoint(Transform* self, const Vector3& point) {
	Vector4 p = GetWorldToLocalMatrix(self) * Vector4(point.x, point.y, point.z, 1);
	return Vector3(p.x, p.y, p.z);
}

Vector3 TransformInternal::InverseTransformDirection(Transform* self, const Vector3& direction) {
	Vector4 d = GetWorldToLocalMatrix(self) * Vector4(direction.x, direction.y, direction.z, 0);
	return Vector3(d.x, d.y, d.z);
}

Transform* TransformInternal::FindChild(const std::string& path) {
	const char* back = path.c_str(), *fwd = back;

	TransformInternal* current = this;
	for (; (fwd = strchr(back, '/')) != nullptr; back = fwd + 1) {
		Transform* child = current->FindDirectChild(std::string(back, fwd));
		if (!child) {
			return nullptr;
		}
		
		current = _suede_rptr(child);
	}

	return current->FindDirectChild(back);
}


void TransformInternal::SetScale(const Vector3& value) {
	ClearDirty(WorldScale);

	if (world_.scale != value) {
		world_.scale = value;
		SetDirty(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenScales();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(2, 0));
	}
}

void TransformInternal::SetPosition(const Vector3& value) {
	ClearDirty(WorldPosition);
	if (world_.position != value) {
		world_.position = value;
		SetDirty(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenPositions();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(0, 0));
	}
}

void TransformInternal::SetRotation(const Quaternion& value) {
	ClearDirty(WorldRotation);

	if (!Mathf::Approximately(world_.rotation, value)) {
		world_.rotation = value;

		SetDirty(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(1, 0));
	}
}

void TransformInternal::SetEulerAngles(const Vector3& value) {
	ClearDirty(WorldEulerAngles);

	if (world_.eulerAngles != value) {
		world_.eulerAngles = value;

		SetDirty(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
		DirtyChildrenRotationsAndEulerAngles();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(1, 0));
	}
}

Vector3 TransformInternal::GetScale(Transform* self) {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		Vector3 scale = GetLocalScale(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			scale *= self->GetScale();
		}

		world_.scale = scale;
		ClearDirty(WorldScale);
	}

	return world_.scale;
}

Vector3 TransformInternal::GetPosition(Transform* self) {
	if (IsDirty(WorldPosition)) {
		if (IsDirty(LocalPosition)) {
			Debug::LogError("invalid state");
		}

		Vector3 position = GetLocalPosition(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			position = self->TransformPoint(position);
		}

		world_.position = position;
		ClearDirty(WorldPosition);
	}

	return world_.position;
}

Quaternion TransformInternal::GetRotation(Transform* self) {
	if (!IsDirty(WorldRotation)) { return world_.rotation; }

	if (!IsDirty(WorldEulerAngles)) {
		world_.rotation = Quaternion(Vector3(
			Mathf::deg2Rad * world_.eulerAngles.x,
			Mathf::deg2Rad * world_.eulerAngles.y,
			Mathf::deg2Rad * world_.eulerAngles.z)
		);
	}
	else {
		Quaternion localRotation;
		if (!IsDirty(LocalRotation)) {
			localRotation = GetLocalRotation(self);
		}
		else {
			if (IsDirty(LocalEulerAngles)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = Quaternion(Mathf::deg2Rad * local_.eulerAngles);
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

Vector3 TransformInternal::GetEulerAngles(Transform* self) {
	if (!IsDirty(WorldEulerAngles)) { return world_.eulerAngles; }

	Quaternion worldRotation;
	if (!IsDirty(WorldRotation)) {
		worldRotation = GetRotation(self);
	}
	else {
		Quaternion localRotation;

		if (!IsDirty(LocalRotation)) {
			localRotation = GetLocalRotation(self);
		}
		else {
			if (IsDirty(LocalEulerAngles)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = Quaternion(Mathf::deg2Rad * local_.eulerAngles);
			ClearDirty(LocalRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			localRotation = self->GetRotation() * localRotation;
		}

		worldRotation = world_.rotation = localRotation;
		ClearDirty(WorldRotation);
	}

	world_.eulerAngles = Mathf::rad2Deg * worldRotation.GetEulerAngles();

	ClearDirty(WorldEulerAngles);

	return world_.eulerAngles;
}

void TransformInternal::SetLocalScale(const Vector3& value) {
	ClearDirty(LocalScale);
	if (local_.scale != value) {
		local_.scale = value;
		SetDirty(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenScales();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(2, 1));
	}
}

void TransformInternal::SetLocalPosition(const Vector3& value) {
	ClearDirty(LocalPosition);
	if (local_.position != value) {
		local_.position = value;
		SetDirty(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
		DirtyChildrenPositions();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(0, 1));
	}
}

void TransformInternal::SetLocalRotation(const Quaternion& value) {
	ClearDirty(LocalRotation);
	if (!Mathf::Approximately(Quaternion::Dot(local_.rotation, value), 0)) {
		local_.rotation = value;
		SetDirty(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(1, 1));
	}
}

void TransformInternal::SetLocalEulerAngles(const Vector3& value) {
	ClearDirty(LocalEulerAngles);
	if (local_.eulerAngles != value) {
		local_.eulerAngles = value;
		SetDirty(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);

		DirtyChildrenRotationsAndEulerAngles();
		gameObject_->RecalculateBounds();

		GameObject::transformChanged.delay_raise(gameObject_, Mathf::MakeDword(1, 1));
	}
}

Vector3 TransformInternal::GetLocalScale(Transform* self) {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		Vector3 scale = GetScale(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			scale /= self->GetScale();
		}

		local_.scale = scale;
		ClearDirty(LocalScale);
	}

	return local_.scale;
}

Vector3 TransformInternal::GetLocalPosition(Transform* self) {
	if (IsDirty(LocalPosition)) {
		if (IsDirty(WorldPosition)) {
			Debug::LogError("invalid state");
		}

		Vector3 position = GetPosition(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			position = self->InverseTransformPoint(position);
		}

		local_.position = position;
		ClearDirty(LocalPosition);
	}

	return local_.position;
}

Quaternion TransformInternal::GetLocalRotation(Transform* self) {
	if (!IsDirty(LocalRotation)) { return local_.rotation; }

	if (!IsDirty(LocalEulerAngles)) {
		local_.rotation = Quaternion(Mathf::deg2Rad * local_.eulerAngles);
	}
	else {
		Quaternion worldRotation;
		if (!IsDirty(WorldRotation)) {
			worldRotation = GetRotation(self);
		}
		else {
			if (IsDirty(WorldEulerAngles)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = Quaternion(Mathf::deg2Rad * world_.eulerAngles);
			ClearDirty(WorldRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			worldRotation = self->GetRotation().GetInversed() * worldRotation;
		}

		local_.rotation = worldRotation;
	}

	ClearDirty(LocalRotation);

	return local_.rotation;
}

Vector3 TransformInternal::GetLocalEulerAngles(Transform* self) {
	if (!IsDirty(LocalEulerAngles)) { return local_.eulerAngles; }

	Quaternion localRotation;
	if (!IsDirty(LocalRotation)) {
		localRotation = GetLocalRotation(self);
	}
	else {
		Quaternion worldRotation;

		if (!IsDirty(WorldRotation)) {
			worldRotation = GetRotation(self);
		}
		else {
			if (IsDirty(WorldEulerAngles)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = Quaternion(Mathf::deg2Rad * world_.eulerAngles);
			ClearDirty(WorldRotation);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			worldRotation = self->GetRotation().GetInversed() * worldRotation;
		}

		localRotation = local_.rotation = worldRotation;
		ClearDirty(LocalRotation);
	}

	Vector3 angles = localRotation.GetEulerAngles();
	local_.eulerAngles = Mathf::rad2Deg * angles;

	ClearDirty(LocalEulerAngles);

	return local_.eulerAngles;
}

Matrix4 TransformInternal::GetLocalToWorldMatrix(Transform* self) {
	if (IsDirty(LocalToWorldMatrix)) {
		Matrix4 matrix = Matrix4::TRS(GetLocalPosition(self), GetLocalRotation(self), GetLocalScale(self));
		if (!IsNullOrRoot(self = self->GetParent())) {
			matrix = self->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		ClearDirty(LocalToWorldMatrix);

		GetGameObject()->SendMessage(GameObjectMessageLocalToWorldMatrixModified, nullptr);
	}

	return localToWorldMatrix_;
}

Matrix4 TransformInternal::GetWorldToLocalMatrix(Transform* self) {
	if (IsDirty(WorldToLocalMatrix)) {
		worldToLocalMatrix_ = GetLocalToWorldMatrix(self).GetInversed();
		ClearDirty(WorldToLocalMatrix);
	}

	return worldToLocalMatrix_;
}

Vector3 TransformInternal::GetLocalToWorldPosition(Transform* self, const Vector3& position) {
	Vector4 p = GetLocalToWorldMatrix(self) * Vector4(position.x, position.y, position.z, 1);
	return Vector3(p.x, p.y, p.z);
}

Vector3 TransformInternal::GetWorldToLocalPosition(Transform* self, const Vector3& position) {
	Vector4 p = GetWorldToLocalMatrix(self) * Vector4(position.x, position.y, position.z, 1);
	return Vector3(p.x, p.y, p.z);
}

Vector3 TransformInternal::GetUp(Transform* self) {
	return GetRotation(self) * Vector3(0, 1, 0);
}

Vector3 TransformInternal::GetRight(Transform* self) {
	return GetRotation(self) * Vector3(1, 0, 0);
}

Vector3 TransformInternal::GetForward(Transform* self) {
	return GetRotation(self) * Vector3(0, 0, -1);
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
	for (int i = 0; i < GetChildCount(); ++i) {
		ref_ptr<Transform>& transform = children_[i];
		transform->GetLocalScale();

		TransformInternal* child = _suede_rptr(transform.get());
		child->SetDirty(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
		child->DirtyChildrenScales();
	}
}

void TransformInternal::DirtyChildrenPositions() {
	for (int i = 0; i < GetChildCount(); ++i) {
		ref_ptr<Transform>& transform = children_[i];
		transform->GetLocalPosition();

		TransformInternal* child = _suede_rptr(transform);
		child->SetDirty(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
		child->DirtyChildrenPositions();
	}
}

void TransformInternal::DirtyChildrenRotationsAndEulerAngles() {
	for (int i = 0; i < GetChildCount(); ++i) {
		ref_ptr<Transform>& transform = children_[i];
		transform->GetLocalRotation();
		transform->GetLocalEulerAngles();
		TransformInternal* child = _suede_rptr(transform.get());
		child->SetDirty(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
		child->DirtyChildrenRotationsAndEulerAngles();
	}
}

bool TransformInternal::IsNullOrRoot(Transform* transform) {
	return !transform || transform == gameObject_->GetScene()->GetRootTransform();
}

Transform* TransformInternal::FindDirectChild(const std::string& name) {
	for (int i = 0; i < children_.size(); ++i) {
		if (name == children_[i]->GetGameObject()->GetName()) {
			return children_[i].get();
		}
	}

	return nullptr;
}

void TransformInternal::ChangeParent(Transform* self, Transform* oldParent, Transform* newParent) {
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
		GameObject::parentChanged.delay_raise(self->GetGameObject());
	}
}

bool TransformInternal::AddChildItem(Children & children, Transform* child) {
	if (std::find(children.begin(), children.end(), child) == children.end()) {
		children.push_back(child);
		return true;
	}

	return false;
}

bool TransformInternal::RemoveChildItem(Children& children, Transform* child) {
	Children::iterator pos = std::find(children.begin(), children.end(), child);
	if (pos != children.end()) {
		children.erase(pos);
		return true;
	}

	return false;
}
