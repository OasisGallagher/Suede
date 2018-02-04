#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "tagmanager.h"
#include "tools/math2.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/entityinternal.h"

EntityInternal::EntityInternal() : EntityInternal(ObjectTypeEntity) {
}

EntityInternal::EntityInternal(ObjectType entityType)
	: ObjectInternal(entityType), dirtyFlag_(0), activeSelf_(true) {
	if (entityType < ObjectTypeEntity || entityType >= ObjectTypeCount) {
		Debug::LogError("invalid entity type %d.", entityType);
	}

	name_ = EntityTypeToString(GetType());
}

bool EntityInternal::GetActive() const {
	return activeSelf_ && GetParent()->GetActiveSelf();
}

void EntityInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;

		EntityActiveEvent e;
		e.entity = dsp_cast<Entity>(shared_from_this());
		WorldInstance()->FireEvent(&e);
	}
}

bool EntityInternal::SetTag(const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		EntityTagChangedEvent e;
		e.entity = dsp_cast<Entity>(shared_from_this());
		WorldInstance()->FireEvent(&e);
	}

	return true;
}

void EntityInternal::SetName(const std::string& value) {
	if (value.empty()) {
		Debug::LogError("empty name.");
		return;
	}

	if (name_ != value) {
		name_ = value;

		EntityNameChangedEvent e;
		e.entity = dsp_cast<Entity>(shared_from_this());
		WorldInstance()->FireEvent(&e);
	}
}

void EntityInternal::AddChild(Entity child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		children_.push_back(child);
		child->SetParent(dsp_cast<Entity>(shared_from_this()));
	}
}

void EntityInternal::RemoveChild(Entity child) {
	std::vector<Entity>::iterator pos = std::find(children_.begin(), children_.end(), child);
	if (pos != children_.end()) {
		child->SetParent(WorldInstance()->GetRootEntity());
		children_.erase(pos);
	}
}

void EntityInternal::RemoveChildAt(uint index) {
	if (index >= children_.size()) {
		Debug::LogError("index out of range.");
		return;
	}

	Entity child = children_[index];
	child->SetParent(WorldInstance()->GetRootEntity());
	children_.erase(children_.begin() + index);
}

void EntityInternal::SetParent(Entity value) {
	if (value->GetInstanceID() == GetInstanceID()) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	std::weak_ptr<Entity::element_type> old = parent_;
	// Save old parent.
	Entity entity = old.lock();
	if (entity == value) { return; }

	Entity thisSp = dsp_cast<Entity>(shared_from_this());
	if (entity) {
		entity->RemoveChild(thisSp);
	}

	parent_ = value;
	entity = parent_.lock();
	if (entity) {
		entity->AddChild(thisSp);
	}

	// Clear dirty flags.
	GetScale();
	GetRotation();
	GetPosition();

	SetDiry(LocalScale | LocalRotation | LocalPosition | LocalEulerAngles);

	EntityParentChangedEvent e;
	e.entity = thisSp;
	WorldInstance()->FireEvent(&e);
}

Entity EntityInternal::FindChild(const std::string& path) {
	const char* back = path.c_str(), *fwd = back;

	EntityInternal* current = this;
	for (; (fwd = strchr(back, '/')) != nullptr; back = fwd + 1) {
		Entity child = current->FindDirectChild(std::string(back, fwd));
		if (!child) {
			return nullptr;
		}

		current = dynamic_cast<EntityInternal*>(child.get());
	}

	return current->FindDirectChild(back);
}

void EntityInternal::Update() {
	if (animation_) {
		animation_->Update();
	}
}

void EntityInternal::SetScale(const glm::vec3& value) {
	ClearDirty(WorldScale);

	if (world_.scale == value) { return; }

	world_.scale = value;
	SetDiry(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenScales();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(2, 0);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void EntityInternal::SetPosition(const glm::vec3& value) {
	ClearDirty(WorldPosition);
	if (world_.position == value) { return; }

	world_.position = value;
	SetDiry(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenPositions();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(0, 0);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void EntityInternal::SetRotation(const glm::quat& value) {
	ClearDirty(WorldRotation);

	if (Math::Approximately(glm::dot(world_.rotation, value), 0)) { return; }
	world_.rotation = value;

	SetDiry(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	DirtyChildrenRotationsAndEulerAngles();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 0);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void EntityInternal::SetEulerAngles(const glm::vec3& value) {
	ClearDirty(WorldEulerAngles);

	if (world_.eulerAngles == value) { return; }
	world_.eulerAngles = value;

	SetDiry(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	DirtyChildrenRotationsAndEulerAngles();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 0);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

glm::vec3 EntityInternal::GetScale() {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		Entity current = dsp_cast<Entity>(shared_from_this());
		glm::vec3 scale = GetLocalScale();
		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			scale *= current->GetScale();
		}

		world_.scale = scale;
		ClearDirty(WorldScale);
	}

	return world_.scale;
}

glm::vec3 EntityInternal::GetPosition() {
	if (IsDirty(WorldPosition)) {
		if (IsDirty(LocalPosition)) {
			Debug::LogError("invalid state");
		}

		Entity current = dsp_cast<Entity>(shared_from_this());
		glm::vec3 position = GetLocalPosition();
		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			position += current->GetPosition();
		}

		world_.position = position;
		ClearDirty(WorldPosition);
	}

	return world_.position;
}

glm::quat EntityInternal::GetRotation() {
	if (!IsDirty(WorldRotation)) { return world_.rotation; }

	if (!IsDirty(WorldEulerAngles)) {
		world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
	}
	else {
		Entity current = dsp_cast<Entity>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			localRotation = current->GetRotation() * localRotation;
		}

		world_.rotation = localRotation;
	}

	ClearDirty(WorldRotation);

	return world_.rotation;
}

glm::vec3 EntityInternal::GetEulerAngles() {
	if (!IsDirty(WorldEulerAngles)) { return world_.eulerAngles; }

	glm::quat worldRotation;
	if (!IsDirty(WorldRotation)) {
		worldRotation = GetRotation();
	}
	else {
		Entity current = dsp_cast<Entity>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			localRotation = current->GetRotation() * localRotation;
		}

		worldRotation = world_.rotation = localRotation;
		ClearDirty(WorldRotation);
	}
	
	world_.eulerAngles = Math::Degrees(glm::eulerAngles(worldRotation));

	ClearDirty(WorldEulerAngles);

	return world_.eulerAngles;
}

void EntityInternal::SetLocalScale(const glm::vec3& value) {
	ClearDirty(LocalScale);
	if (local_.scale == value) { return; }

	local_.scale = value;
	SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenScales();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(2, 1);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void EntityInternal::SetLocalPosition(const glm::vec3& value) {
	ClearDirty(LocalPosition);
	if (local_.position == value) { return; }

	local_.position = value;
	SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	DirtyChildrenPositions();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(0, 1);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void EntityInternal::SetLocalRotation(const glm::quat& value) {
	ClearDirty(LocalRotation);
	if (Math::Approximately(glm::dot(local_.rotation, value), 0)) { return; }

	local_.rotation = value;
	SetDiry(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenRotationsAndEulerAngles();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 1);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void EntityInternal::SetLocalEulerAngles(const glm::vec3& value) {
	ClearDirty(LocalEulerAngles);
	if (local_.eulerAngles == value) { return; }

	local_.eulerAngles = value;
	SetDiry(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenRotationsAndEulerAngles();

	EntityTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 1);
	e.entity = dsp_cast<Entity>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

glm::vec3 EntityInternal::GetLocalScale() {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		Entity current = dsp_cast<Entity>(shared_from_this());
		glm::vec3 scale = GetScale();
		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			scale /= current->GetScale();
		}

		local_.scale = scale;
		ClearDirty(LocalScale);
	}

	return local_.scale;
}

glm::vec3 EntityInternal::GetLocalPosition() {
	if (IsDirty(LocalPosition)) {
		if (IsDirty(WorldPosition)) {
			Debug::LogError("invalid state");
		}

		Entity current = dsp_cast<Entity>(shared_from_this());
		glm::vec3 position = GetPosition();
		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			position -= current->GetPosition();
		}

		local_.position = position;
		ClearDirty(LocalPosition);
	}

	return local_.position;
}

glm::quat EntityInternal::GetLocalRotation() {
	if (!IsDirty(LocalRotation)) { return local_.rotation; }

	if (!IsDirty(LocalEulerAngles)) {
		local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
	}
	else {
		Entity current = dsp_cast<Entity>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			worldRotation = glm::inverse(current->GetRotation()) * worldRotation;
		}

		local_.rotation = worldRotation;
	}

	ClearDirty(LocalRotation);

	return local_.rotation;
}

glm::vec3 EntityInternal::GetLocalEulerAngles() {
	if (!IsDirty(LocalEulerAngles)) { return local_.eulerAngles; }

	glm::quat localRotation;
	if (!IsDirty(LocalRotation)) {
		localRotation = GetLocalRotation();
	}
	else {
		Entity current = dsp_cast<Entity>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
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

glm::mat4 EntityInternal::GetLocalToWorldMatrix() {
	if (IsDirty(LocalToWorldMatrix)) {
		Entity current = dsp_cast<Entity>(shared_from_this());
		glm::mat4 matrix = TRS(GetLocalPosition(), GetLocalRotation(), GetLocalScale());
		if ((current = current->GetParent()) != WorldInstance()->GetRootEntity()) {
			matrix = current->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		ClearDirty(LocalToWorldMatrix);
	}

	return localToWorldMatrix_;
}

glm::mat4 EntityInternal::GetWorldToLocalMatrix() {
	if (IsDirty(WorldToLocalMatrix)) {
		worldToLocalMatrix_ = glm::inverse(GetLocalToWorldMatrix());
		ClearDirty(WorldToLocalMatrix);
	}

	return worldToLocalMatrix_;
}

glm::vec3 EntityInternal::GetLocalToWorldPosition(const glm::vec3& position) {
	return glm::vec3(GetLocalToWorldMatrix() * glm::vec4(position, 1));
}

glm::vec3 EntityInternal::GetWorldToLocalPosition(const glm::vec3& position) {
	return glm::vec3(GetWorldToLocalMatrix() * glm::vec4(position, 1));
}

glm::vec3 EntityInternal::GetUp() {
	return local_.rotation * glm::vec3(0, 1, 0);
}

glm::vec3 EntityInternal::GetRight() {
	return local_.rotation * glm::vec3(1, 0, 0);
}

glm::vec3 EntityInternal::GetForward() {
	return local_.rotation * glm::vec3(0, 0, -1);
}

void EntityInternal::SetDiry(int bits) {
	dirtyFlag_ |= bits;
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

void EntityInternal::DirtyChildrenScales() {
	for (int i = 0; i < GetChildCount(); ++i) {
		EntityInternal* si = dynamic_cast<EntityInternal*>(GetChildAt(i).get());
		si->GetLocalScale();
		si->SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void EntityInternal::DirtyChildrenPositions() {
	for (int i = 0; i < GetChildCount(); ++i) {
		EntityInternal* si = dynamic_cast<EntityInternal*>(GetChildAt(i).get());
		si->GetLocalPosition();
		si->SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void EntityInternal::DirtyChildrenRotationsAndEulerAngles() {
	for (int i = 0; i < GetChildCount(); ++i) {
		EntityInternal* si = dynamic_cast<EntityInternal*>(GetChildAt(i).get());
		si->GetLocalRotation();
		si->GetLocalEulerAngles();
		si->SetDiry(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

Entity EntityInternal::FindDirectChild(const std::string& name) {
	for (int i = 0; i < children_.size(); ++i) {
		if (name == children_[i]->GetName()) {
			return children_[i];
		}
	}

	return nullptr;
}

glm::mat4 EntityInternal::TRS(const glm::vec3& t, const glm::quat& r, const glm::vec3& s) {
	return glm::translate(glm::mat4(1), t) * glm::scale(glm::mat4_cast(r), GetLocalScale());
}

const char* EntityInternal::EntityTypeToString(ObjectType type) {
	const char* name = "";
	switch (type) {
		case ObjectTypeEntity:
			name = "Entity";
			break;
		case ObjectTypeCamera:
			name = "Camera";
			break;
		case ObjectTypeSkybox:
			name = "Skybox";
			break;
		case ObjectTypeSpotLight:
			name = "SpotLight";
			break;
		case ObjectTypePointLight:
			name = "PointLight";
			break;
		case ObjectTypeDirectionalLight:
			name = "DirectionalLight";
			break;
		case ObjectTypeParticleSystem:
			name = "ParticleSystem";
			break;
		default:
			Debug::LogError("entity name for %d does not exist.", type);
			break;
	}

	return name;
}
