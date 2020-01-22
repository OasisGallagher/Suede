#include "scene.h"
#include "math/mathf.h"
#include "transforminternal.h"

Transform::Transform() : Component(new TransformInternal) {}
bool Transform::IsAttachedToScene() { return _suede_dptr()->IsAttachedToScene(this); }
void Transform::TraversalHierarchy(std::function<TraversalCommand(Transform*)> func) { _suede_dptr()->TraversalHierarchy(this, func); }
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
void Transform::LookAt(const Vector3& target, const Vector3& up) { _suede_dptr()->LookAt(this, target, up); }
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

event<Transform*, bool> TransformInternal::attached;

SUEDE_DEFINE_COMPONENT_INTERNAL(Transform, Component)

TransformInternal::TransformInternal() : ComponentInternal(ObjectType::Transform), parent_(nullptr) {
	local_.scale = world_.scale = Vector3(1);
}

TransformInternal::~TransformInternal() {
}

void TransformInternal::TraversalHierarchy(Transform* self, std::function<TraversalCommand(Transform*)> func) {
	if (self == gameObject_->GetScene()->GetRootTransform()) {
		DFSRecursively(self, func);
	}
	else if (func(self) == TraversalCommand::Continue) {
		DFSRecursively(self, func);
	}
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
	SUEDE_ASSERT(index < children_.size());
	Transform* child = children_[index].get();
	RemoveChild(child);
}

void TransformInternal::SetParent(Transform* self, Transform* value) {
	if (self == value) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	if (parent_ != value) {
		ChangeParent(self, parent_, value);
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
		
		current = _suede_drptr(child);
	}

	return current->FindDirectChild(back);
}


void TransformInternal::LookAt(Transform* self, const Vector3& target, const Vector3& up) {
	Matrix4 mat = Matrix4::Translate(world_.position) * Matrix4::LookAt(GetPosition(self), target, up);
	Vector3 skew, scale, translation;
	Vector4 perspective;
	Quaternion orientation;
	if (Matrix4::Decompose(mat, scale, orientation, translation, skew, perspective)) {
		worldToLocalMatrix_ = mat;
		localToWorldMatrix_ = mat.GetInversed();

		world_.position = translation;
		world_.scale = scale;
		world_.rotation = orientation;
		world_.eulerAngles = orientation.GetEulerAngles();

		SetDirty(WorldScaleMask | WorldRotationMask | WorldEulerAnglesMask | WorldPositionMask, false);
		SetDirty(LocalScaleMask | LocalRotationMask | LocalEulerAnglesMask | LocalPositionMask, true);
	}
	else {
		Debug::LogError("failed to decompose look at matrix, invalid parameters");
	}
}

void TransformInternal::SetScale(const Vector3& value) {
	SetDirty(WorldScaleMask ,false);

	if (world_.scale != value) {
		world_.scale = value;
		SetDirty(LocalScaleMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);

		DirtyChildrenScales();
	}
}

void TransformInternal::SetPosition(const Vector3& value) {
	SetDirty(WorldPositionMask, false);
	if (world_.position != value) {
		world_.position = value;
		SetDirty(LocalPositionMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);

		DirtyChildrenPositions();
	}
}

void TransformInternal::SetRotation(const Quaternion& value) {
	SetDirty(WorldRotationMask, false);

	if (world_.rotation != value) {
		world_.rotation = value;

		SetDirty(LocalRotationMask | LocalEulerAnglesMask | WorldEulerAnglesMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);

		DirtyChildrenRotationsAndEulerAngles();
	}
}

void TransformInternal::SetEulerAngles(const Vector3& value) {
	SetDirty(WorldEulerAnglesMask, false);

	if (world_.eulerAngles != value) {
		world_.eulerAngles = value;

		SetDirty(WorldRotationMask | LocalRotationMask | LocalEulerAnglesMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);
		DirtyChildrenRotationsAndEulerAngles();
	}
}

Vector3 TransformInternal::GetScale(Transform* self) {
	if (IsDirty(WorldScaleMask)) {
		if (IsDirty(LocalScaleMask)) {
			Debug::LogError("invalid state");
		}

		Vector3 scale = GetLocalScale(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			scale *= self->GetScale();
		}

		world_.scale = scale;
		SetDirty(WorldScaleMask, false);
	}

	return world_.scale;
}

Vector3 TransformInternal::GetPosition(Transform* self) {
	if (IsDirty(WorldPositionMask)) {
		if (IsDirty(LocalPositionMask)) {
			Debug::LogError("invalid state");
		}

		Vector3 position = GetLocalPosition(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			position = self->TransformPoint(position);
		}

		world_.position = position;
		SetDirty(WorldPositionMask, false);
	}

	return world_.position;
}

Quaternion TransformInternal::GetRotation(Transform* self) {
	if (!IsDirty(WorldRotationMask)) { return world_.rotation; }

	if (!IsDirty(WorldEulerAnglesMask)) {
		world_.rotation = Quaternion(Vector3(
			Mathf::deg2Rad * world_.eulerAngles.x,
			Mathf::deg2Rad * world_.eulerAngles.y,
			Mathf::deg2Rad * world_.eulerAngles.z)
		);
	}
	else {
		Quaternion localRotation;
		if (!IsDirty(LocalRotationMask)) {
			localRotation = GetLocalRotation(self);
		}
		else {
			if (IsDirty(LocalEulerAnglesMask)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = Quaternion(Mathf::deg2Rad * local_.eulerAngles);
			SetDirty(LocalRotationMask, false);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			localRotation = self->GetRotation() * localRotation;
		}

		world_.rotation = localRotation;
	}

	SetDirty(WorldRotationMask, false);

	return world_.rotation;
}

Vector3 TransformInternal::GetEulerAngles(Transform* self) {
	if (!IsDirty(WorldEulerAnglesMask)) { return world_.eulerAngles; }

	Quaternion worldRotation;
	if (!IsDirty(WorldRotationMask)) {
		worldRotation = GetRotation(self);
	}
	else {
		Quaternion localRotation;

		if (!IsDirty(LocalRotationMask)) {
			localRotation = GetLocalRotation(self);
		}
		else {
			if (IsDirty(LocalEulerAnglesMask)) {
				Debug::LogError("invalid state");
			}

			localRotation = local_.rotation = Quaternion(Mathf::deg2Rad * local_.eulerAngles);
			SetDirty(LocalRotationMask, false);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			localRotation = self->GetRotation() * localRotation;
		}

		worldRotation = world_.rotation = localRotation;
		SetDirty(WorldRotationMask, false);
	}

	world_.eulerAngles = Mathf::rad2Deg * worldRotation.GetEulerAngles();

	SetDirty(WorldEulerAnglesMask, false);

	return world_.eulerAngles;
}

void TransformInternal::SetLocalScale(const Vector3& value) {
	SetDirty(LocalScaleMask, false);
	if (local_.scale != value) {
		local_.scale = value;
		SetDirty(WorldScaleMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);

		DirtyChildrenScales();
	}
}

void TransformInternal::SetLocalPosition(const Vector3& value) {
	SetDirty(LocalPositionMask, false);
	if (local_.position != value) {
		local_.position = value;
		SetDirty(WorldPositionMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);
		DirtyChildrenPositions();
	}
}

void TransformInternal::SetLocalRotation(const Quaternion& value) {
	SetDirty(LocalRotationMask, false);
	if (!Mathf::Approximately(Quaternion::Dot(local_.rotation, value), 0)) {
		local_.rotation = value;
		SetDirty(WorldRotationMask | LocalEulerAnglesMask | WorldEulerAnglesMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);

		DirtyChildrenRotationsAndEulerAngles();
	}
}

void TransformInternal::SetLocalEulerAngles(const Vector3& value) {
	SetDirty(LocalEulerAnglesMask, false);
	if (local_.eulerAngles != value) {
		local_.eulerAngles = value;
		SetDirty(WorldEulerAnglesMask | LocalRotationMask | WorldRotationMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);

		DirtyChildrenRotationsAndEulerAngles();
	}
}

Vector3 TransformInternal::GetLocalScale(Transform* self) {
	if (IsDirty(LocalScaleMask)) {
		if (IsDirty(WorldScaleMask)) {
			Debug::LogError("invalid state");
		}

		Vector3 scale = GetScale(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			scale /= self->GetScale();
		}

		local_.scale = scale;
		SetDirty(LocalScaleMask, false);
	}

	return local_.scale;
}

Vector3 TransformInternal::GetLocalPosition(Transform* self) {
	if (IsDirty(LocalPositionMask)) {
		if (IsDirty(WorldPositionMask)) {
			Debug::LogError("invalid state");
		}

		Vector3 position = GetPosition(self);
		if (!IsNullOrRoot(self = self->GetParent())) {
			position = self->InverseTransformPoint(position);
		}

		local_.position = position;
		SetDirty(LocalPositionMask, false);
	}

	return local_.position;
}

Quaternion TransformInternal::GetLocalRotation(Transform* self) {
	if (!IsDirty(LocalRotationMask)) { return local_.rotation; }

	if (!IsDirty(LocalEulerAnglesMask)) {
		local_.rotation = Quaternion(Mathf::deg2Rad * local_.eulerAngles);
	}
	else {
		Quaternion worldRotation;
		if (!IsDirty(WorldRotationMask)) {
			worldRotation = GetRotation(self);
		}
		else {
			if (IsDirty(WorldEulerAnglesMask)) {
				Debug::LogError("invalid state");
			}

			worldRotation = world_.rotation = Quaternion(Mathf::deg2Rad * world_.eulerAngles);
			SetDirty(WorldRotationMask, false);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			worldRotation = self->GetRotation().GetInversed() * worldRotation;
		}

		local_.rotation = worldRotation;
	}

	SetDirty(LocalRotationMask, false);

	return local_.rotation;
}

Vector3 TransformInternal::GetLocalEulerAngles(Transform* self) {
	if (!IsDirty(LocalEulerAnglesMask)) { return local_.eulerAngles; }

	Quaternion localRotation;
	if (!IsDirty(LocalRotationMask)) {
		localRotation = GetLocalRotation(self);
	}
	else {
		Quaternion worldRotation;

		if (!IsDirty(WorldRotationMask)) {
			worldRotation = GetRotation(self);
		}
		else {
			SUEDE_ASSERT(!IsDirty(WorldEulerAnglesMask));

			worldRotation = world_.rotation = Quaternion(Mathf::deg2Rad * world_.eulerAngles);
			SetDirty(WorldRotationMask, false);
		}

		if (!IsNullOrRoot(self = self->GetParent())) {
			worldRotation = self->GetRotation().GetInversed() * worldRotation;
		}

		localRotation = local_.rotation = worldRotation;
		SetDirty(LocalRotationMask, false);
	}

	Vector3 angles = localRotation.GetEulerAngles();
	local_.eulerAngles = Mathf::rad2Deg * angles;

	SetDirty(LocalEulerAnglesMask, false);

	return local_.eulerAngles;
}

Matrix4 TransformInternal::GetLocalToWorldMatrix(Transform* self) {
	if (IsDirty(LocalToWorldMatrixMask)) {
		Matrix4 matrix = Matrix4::TRS(GetLocalPosition(self), GetLocalRotation(self), GetLocalScale(self));
		if (!IsNullOrRoot(self = self->GetParent())) {
			matrix = self->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		SetDirty(LocalToWorldMatrixMask, false);
	}

	return localToWorldMatrix_;
}

Matrix4 TransformInternal::GetWorldToLocalMatrix(Transform* self) {
	if (IsDirty(WorldToLocalMatrixMask)) {
		worldToLocalMatrix_ = GetLocalToWorldMatrix(self).GetInversed();
		SetDirty(WorldToLocalMatrixMask, false);
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
	return GetRotation(self) * Vector3::up;
}

Vector3 TransformInternal::GetRight(Transform* self) {
	return GetRotation(self) * Vector3::right;
}

Vector3 TransformInternal::GetForward(Transform* self) {
	return GetRotation(self) * Vector3::forward;
}

void TransformInternal::SetDirty(uint bits, bool value) {
	if (value) { dirtyBits_ |= bits; }
	else { dirtyBits_ &= ~bits; }

	SUEDE_ASSERT(!IsDirty(LocalScaleMask) || !IsDirty(WorldScaleMask));
	SUEDE_ASSERT(!IsDirty(LocalPositionMask) || !IsDirty(WorldPositionMask));
	SUEDE_ASSERT(!IsDirty(LocalRotationMask) || !IsDirty(WorldRotationMask) || !IsDirty(LocalEulerAnglesMask) || !IsDirty(WorldEulerAnglesMask));
}

void TransformInternal::DirtyChildrenScales() {
	for (int i = 0; i < GetChildCount(); ++i) {
		ref_ptr<Transform>& transform = children_[i];
		transform->GetLocalScale();

		TransformInternal* child = _suede_drptr(transform.get());
		child->SetDirty(WorldScaleMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);
		child->DirtyChildrenScales();
	}
}

void TransformInternal::DirtyChildrenPositions() {
	for (int i = 0; i < GetChildCount(); ++i) {
		ref_ptr<Transform>& transform = children_[i];
		transform->GetLocalPosition();

		TransformInternal* child = _suede_drptr(transform.get());
		child->SetDirty(WorldPositionMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);
		child->DirtyChildrenPositions();
	}
}

void TransformInternal::DirtyChildrenRotationsAndEulerAngles() {
	for (int i = 0; i < GetChildCount(); ++i) {
		ref_ptr<Transform>& transform = children_[i];
		transform->GetLocalRotation();
		transform->GetLocalEulerAngles();
		TransformInternal* child = _suede_drptr(transform.get());
		child->SetDirty(WorldRotationMask | WorldEulerAnglesMask | LocalToWorldMatrixMask | WorldToLocalMatrixMask, true);
		child->DirtyChildrenRotationsAndEulerAngles();
	}
}

bool TransformInternal::DFSRecursively(Transform* root, std::function<TraversalCommand(Transform*)> func) {
	for (int i = 0; i < root->GetChildCount(); ++i) {
		Transform* child = root->GetChildAt(i);
		if (child == nullptr) {
			continue;
		}

		TraversalCommand command = func(child);

		if (command == TraversalCommand::NextSibling) {
			continue;
		}

		if (command == TraversalCommand::Break) {
			return false;
		}

		if (!DFSRecursively(child->GetTransform(), func)) {
			return false;
		}
	}

	return true;
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
	if (oldParent != nullptr) { // Remove from old parent.
		RemoveChildItem(_suede_drptr(oldParent)->children_, self);
	}

	if (newParent != nullptr) {
		AddChildItem(_suede_drptr(newParent)->children_, self);
	}

	parent_ = newParent;

	// Clear dirty flags.
	GetScale(self);
	GetRotation(self);
	GetPosition(self);
	SetDirty(LocalScaleMask | LocalRotationMask | LocalPositionMask | LocalEulerAnglesMask, true);

	bool oldAttached = (oldParent != nullptr && oldParent->IsAttachedToScene());
	bool newAttached = (newParent != nullptr && newParent->IsAttachedToScene());
	if (oldAttached != newAttached) {
		attached.raise(self, newAttached);
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
