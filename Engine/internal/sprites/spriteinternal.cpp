#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "tagmanager.h"
#include "tools/math2.h"
#include "internal/world/worldinternal.h"
#include "internal/sprites/spriteinternal.h"

SpriteInternal::SpriteInternal() : SpriteInternal(ObjectTypeSprite) {
}

SpriteInternal::SpriteInternal(ObjectType spriteType)
	: ObjectInternal(spriteType), dirtyFlag_(0), activeSelf_(true) {
	if (spriteType < ObjectTypeSprite || spriteType >= ObjectTypeCount) {
		Debug::LogError("invalid sprite type %d.", spriteType);
	}

	name_ = SpriteTypeToString(GetType());
}

bool SpriteInternal::GetActive() const {
	return activeSelf_ && GetParent()->GetActiveSelf();
}

void SpriteInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;

		SpriteActiveEvent e;
		e.sprite = dsp_cast<Sprite>(shared_from_this());
		WorldInstance()->FireEvent(&e);
	}
}

bool SpriteInternal::SetTag(const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		SpriteTagChangedEvent e;
		e.sprite = dsp_cast<Sprite>(shared_from_this());
		WorldInstance()->FireEvent(&e);
	}

	return true;
}

void SpriteInternal::SetName(const std::string& value) {
	if (value.empty()) {
		Debug::LogError("empty name.");
		return;
	}

	if (name_ != value) {
		name_ = value;

		SpriteNameChangedEvent e;
		e.sprite = dsp_cast<Sprite>(shared_from_this());
		WorldInstance()->FireEvent(&e);
	}
}

void SpriteInternal::AddChild(Sprite child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		children_.push_back(child);
		child->SetParent(dsp_cast<Sprite>(shared_from_this()));
	}
}

void SpriteInternal::RemoveChild(Sprite child) {
	std::vector<Sprite>::iterator pos = std::find(children_.begin(), children_.end(), child);
	if (pos != children_.end()) {
		child->SetParent(WorldInstance()->GetRootSprite());
		children_.erase(pos);
	}
}

void SpriteInternal::RemoveChildAt(uint index) {
	if (index >= children_.size()) {
		Debug::LogError("index out of range.");
		return;
	}

	Sprite child = children_[index];
	child->SetParent(WorldInstance()->GetRootSprite());
	children_.erase(children_.begin() + index);
}

void SpriteInternal::SetParent(Sprite value) {
	if (value->GetInstanceID() == GetInstanceID()) {
		Debug::LogError("parent can not be itself.");
		return;
	}

	std::weak_ptr<Sprite::element_type> old = parent_;
	// Save old parent.
	Sprite sprite = old.lock();
	if (sprite == value) { return; }

	Sprite thisSp = dsp_cast<Sprite>(shared_from_this());
	if (sprite) {
		sprite->RemoveChild(thisSp);
	}

	parent_ = value;
	sprite = parent_.lock();
	if (sprite) {
		sprite->AddChild(thisSp);
	}

	// Clear dirty flags.
	GetScale();
	GetRotation();
	GetPosition();

	SetDiry(LocalScale | LocalRotation | LocalPosition | LocalEulerAngles);

	SpriteParentChangedEvent e;
	e.sprite = thisSp;
	WorldInstance()->FireEvent(&e);
}

Sprite SpriteInternal::FindChild(const std::string& path) {
	const char* back = path.c_str(), *fwd = back;

	SpriteInternal* current = this;
	for (; (fwd = strchr(back, '/')) != nullptr; back = fwd + 1) {
		Sprite child = current->FindDirectChild(std::string(back, fwd));
		if (!child) {
			return nullptr;
		}

		current = dynamic_cast<SpriteInternal*>(child.get());
	}

	return current->FindDirectChild(back);
}

void SpriteInternal::Update() {
	if (animation_) {
		animation_->Update();
	}
}

void SpriteInternal::SetScale(const glm::vec3& value) {
	ClearDirty(WorldScale);

	if (world_.scale == value) { return; }

	world_.scale = value;
	SetDiry(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenScales();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(2, 0);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void SpriteInternal::SetPosition(const glm::vec3& value) {
	ClearDirty(WorldPosition);
	if (world_.position == value) { return; }

	world_.position = value;
	SetDiry(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenPositions();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(0, 0);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void SpriteInternal::SetRotation(const glm::quat& value) {
	ClearDirty(WorldRotation);

	if (Math::Approximately(glm::dot(world_.rotation, value), 0)) { return; }
	world_.rotation = value;

	SetDiry(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	DirtyChildrenRotationsAndEulerAngles();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 0);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void SpriteInternal::SetEulerAngles(const glm::vec3& value) {
	ClearDirty(WorldEulerAngles);

	if (world_.eulerAngles == value) { return; }
	world_.eulerAngles = value;

	SetDiry(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	DirtyChildrenRotationsAndEulerAngles();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 0);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

glm::vec3 SpriteInternal::GetScale() {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 scale = GetLocalScale();
		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			scale *= current->GetScale();
		}

		world_.scale = scale;
		ClearDirty(WorldScale);
	}

	return world_.scale;
}

glm::vec3 SpriteInternal::GetPosition() {
	if (IsDirty(WorldPosition)) {
		if (IsDirty(LocalPosition)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 position = GetLocalPosition();
		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			position += current->GetPosition();
		}

		world_.position = position;
		ClearDirty(WorldPosition);
	}

	return world_.position;
}

glm::quat SpriteInternal::GetRotation() {
	if (!IsDirty(WorldRotation)) { return world_.rotation; }

	if (!IsDirty(WorldEulerAngles)) {
		world_.rotation = glm::quat(Math::Radians(world_.eulerAngles));
	}
	else {
		Sprite current = dsp_cast<Sprite>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			localRotation = current->GetRotation() * localRotation;
		}

		world_.rotation = localRotation;
	}

	ClearDirty(WorldRotation);

	return world_.rotation;
}

glm::vec3 SpriteInternal::GetEulerAngles() {
	if (!IsDirty(WorldEulerAngles)) { return world_.eulerAngles; }

	glm::quat worldRotation;
	if (!IsDirty(WorldRotation)) {
		worldRotation = GetRotation();
	}
	else {
		Sprite current = dsp_cast<Sprite>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			localRotation = current->GetRotation() * localRotation;
		}

		worldRotation = world_.rotation = localRotation;
		ClearDirty(WorldRotation);
	}
	
	world_.eulerAngles = Math::Degrees(glm::eulerAngles(worldRotation));

	ClearDirty(WorldEulerAngles);

	return world_.eulerAngles;
}

void SpriteInternal::SetLocalScale(const glm::vec3& value) {
	ClearDirty(LocalScale);
	if (local_.scale == value) { return; }

	local_.scale = value;
	SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenScales();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(2, 1);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void SpriteInternal::SetLocalPosition(const glm::vec3& value) {
	ClearDirty(LocalPosition);
	if (local_.position == value) { return; }

	local_.position = value;
	SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	DirtyChildrenPositions();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(0, 1);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void SpriteInternal::SetLocalRotation(const glm::quat& value) {
	ClearDirty(LocalRotation);
	if (Math::Approximately(glm::dot(local_.rotation, value), 0)) { return; }

	local_.rotation = value;
	SetDiry(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenRotationsAndEulerAngles();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 1);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

void SpriteInternal::SetLocalEulerAngles(const glm::vec3& value) {
	ClearDirty(LocalEulerAngles);
	if (local_.eulerAngles == value) { return; }

	local_.eulerAngles = value;
	SetDiry(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);

	DirtyChildrenRotationsAndEulerAngles();

	SpriteTransformChangedEvent e;
	e.prs = Math::MakeDword(1, 1);
	e.sprite = dsp_cast<Sprite>(shared_from_this());
	WorldInstance()->FireEvent(&e);
}

glm::vec3 SpriteInternal::GetLocalScale() {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 scale = GetScale();
		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			scale /= current->GetScale();
		}

		local_.scale = scale;
		ClearDirty(LocalScale);
	}

	return local_.scale;
}

glm::vec3 SpriteInternal::GetLocalPosition() {
	if (IsDirty(LocalPosition)) {
		if (IsDirty(WorldPosition)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 position = GetPosition();
		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			position -= current->GetPosition();
		}

		local_.position = position;
		ClearDirty(LocalPosition);
	}

	return local_.position;
}

glm::quat SpriteInternal::GetLocalRotation() {
	if (!IsDirty(LocalRotation)) { return local_.rotation; }

	if (!IsDirty(LocalEulerAngles)) {
		local_.rotation = glm::quat(Math::Radians(local_.eulerAngles));
	}
	else {
		Sprite current = dsp_cast<Sprite>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			worldRotation = glm::inverse(current->GetRotation()) * worldRotation;
		}

		local_.rotation = worldRotation;
	}

	ClearDirty(LocalRotation);

	return local_.rotation;
}

glm::vec3 SpriteInternal::GetLocalEulerAngles() {
	if (!IsDirty(LocalEulerAngles)) { return local_.eulerAngles; }

	glm::quat localRotation;
	if (!IsDirty(LocalRotation)) {
		localRotation = GetLocalRotation();
	}
	else {
		Sprite current = dsp_cast<Sprite>(shared_from_this());
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

		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
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

glm::mat4 SpriteInternal::GetLocalToWorldMatrix() {
	if (IsDirty(LocalToWorldMatrix)) {
		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::mat4 matrix = TRS(GetLocalPosition(), GetLocalRotation(), GetLocalScale());
		if ((current = current->GetParent()) != WorldInstance()->GetRootSprite()) {
			matrix = current->GetLocalToWorldMatrix() * matrix;
		}

		localToWorldMatrix_ = matrix;
		ClearDirty(LocalToWorldMatrix);
	}

	return localToWorldMatrix_;
}

glm::mat4 SpriteInternal::GetWorldToLocalMatrix() {
	if (IsDirty(WorldToLocalMatrix)) {
		worldToLocalMatrix_ = glm::inverse(GetLocalToWorldMatrix());
		ClearDirty(WorldToLocalMatrix);
	}

	return worldToLocalMatrix_;
}

glm::vec3 SpriteInternal::GetLocalToWorldPosition(const glm::vec3& position) {
	return glm::vec3(GetLocalToWorldMatrix() * glm::vec4(position, 1));
}

glm::vec3 SpriteInternal::GetWorldToLocalPosition(const glm::vec3& position) {
	return glm::vec3(GetWorldToLocalMatrix() * glm::vec4(position, 1));
}

glm::vec3 SpriteInternal::GetUp() {
	return local_.rotation * glm::vec3(0, 1, 0);
}

glm::vec3 SpriteInternal::GetRight() {
	return local_.rotation * glm::vec3(1, 0, 0);
}

glm::vec3 SpriteInternal::GetForward() {
	return local_.rotation * glm::vec3(0, 0, -1);
}

void SpriteInternal::SetDiry(int bits) {
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

void SpriteInternal::DirtyChildrenScales() {
	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalScale();
		si->SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::DirtyChildrenPositions() {
	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalPosition();
		si->SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::DirtyChildrenRotationsAndEulerAngles() {
	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalRotation();
		si->GetLocalEulerAngles();
		si->SetDiry(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

Sprite SpriteInternal::FindDirectChild(const std::string& name) {
	for (int i = 0; i < children_.size(); ++i) {
		if (name == children_[i]->GetName()) {
			return children_[i];
		}
	}

	return nullptr;
}

glm::mat4 SpriteInternal::TRS(const glm::vec3& t, const glm::quat& r, const glm::vec3& s) {
	return glm::translate(glm::mat4(1), t) * glm::scale(glm::mat4_cast(r), GetLocalScale());
}

const char* SpriteInternal::SpriteTypeToString(ObjectType type) {
	const char* name = "";
	switch (type) {
		case ObjectTypeSprite:
			name = "Sprite";
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
			Debug::LogError("sprite name for %d does not exist.", type);
			break;
	}

	return name;
}
