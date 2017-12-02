#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "math2.h"
#include "tools/string.h"
#include "internal/file/assetimporter.h"
#include "internal/world/worldinternal.h"
#include "internal/sprites/spriteinternal.h"

SpriteInternal::SpriteInternal() : SpriteInternal(ObjectTypeSprite) {
}

SpriteInternal::SpriteInternal(ObjectType spriteType)
	: ObjectInternal(spriteType), dirtyFlag_(0), localScale_(1), worldScale_(1), active_(true) {
	if (spriteType < ObjectTypeSprite || spriteType >= ObjectTypeCount) {
		Debug::LogError("invalid sprite type %d.", spriteType);
	}

	name_ = SpriteTypeToString(GetType());
}

void SpriteInternal::AddChild(Sprite child) {
	if (std::find(children_.begin(), children_.end(), child) == children_.end()) {
		children_.push_back(child);
		child->SetParent(dsp_cast<Sprite>(shared_from_this()));
	}
}

void SpriteInternal::RemoveChild(Sprite child) {
	std::vector<Sprite>::iterator pos = std::find(children_.begin(), children_.end(), child);
	if(pos != children_.end()) {
		child->SetParent(worldInstance->GetRootSprite());
		children_.erase(pos);
	}
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

	if (worldScale_ == value) { return; }

	worldScale_ = value;
	SetDiry(LocalScale | LocalToWorldMatrix | WorldToLocalMatrix);

	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalScale();
		si->SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::SetPosition(const glm::vec3& value) {
	ClearDirty(WorldPosition);
	if (worldPosition_ == value) { return; }

	worldPosition_ = value;
	SetDiry(LocalPosition | LocalToWorldMatrix | WorldToLocalMatrix);

	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalPosition();
		si->SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::SetRotation(const glm::quat& value) {
	ClearDirty(WorldRotation);

	if (Math::Approximately(glm::dot(worldRotation_, value), 0)) { return; }
	worldRotation_ = value;

	SetDiry(LocalRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalRotation();
		si->GetLocalEulerAngles();
		si->SetDiry(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::SetEulerAngles(const glm::vec3& value) {
	ClearDirty(WorldEulerAngles);

	if (worldEulerAngles_ == value) { return; }
	worldEulerAngles_ = value;

	SetDiry(WorldRotation | LocalRotation | LocalEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	for (int i = 0; i < GetChildCount(); ++i) {
		SpriteInternal* si = dynamic_cast<SpriteInternal*>(GetChildAt(i).get());
		si->GetLocalRotation();
		si->GetLocalEulerAngles();
		si->SetDiry(WorldRotation | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

glm::vec3 SpriteInternal::GetScale() {
	if (IsDirty(WorldScale)) {
		if (IsDirty(LocalScale)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 scale = GetLocalScale();
		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			scale *= current->GetScale();
		}

		worldScale_ = scale;
		ClearDirty(WorldScale);
	}

	return worldScale_;
}

glm::vec3 SpriteInternal::GetPosition() {
	if (IsDirty(WorldPosition)) {
		if (IsDirty(LocalPosition)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 position = GetLocalPosition();
		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			position += current->GetPosition();
		}

		worldPosition_ = position;
		ClearDirty(WorldPosition);
	}

	return worldPosition_;
}

glm::quat SpriteInternal::GetRotation() {
	if (!IsDirty(WorldRotation)) { return worldRotation_; }

	if (!IsDirty(WorldEulerAngles)) {
		worldRotation_ = glm::quat(Math::Radians(worldEulerAngles_));
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

			localRotation = localRotation_ = glm::quat(Math::Radians(localEulerAngles_));
			ClearDirty(LocalRotation);
		}

		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			localRotation *= current->GetRotation();
		}

		worldRotation_ = localRotation;
	}

	ClearDirty(WorldRotation);

	return worldRotation_;
}

glm::vec3 SpriteInternal::GetEulerAngles() {
	if (!IsDirty(WorldEulerAngles)) { return worldEulerAngles_; }

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

			localRotation = localRotation_ = glm::quat(Math::Radians(localEulerAngles_));
			ClearDirty(LocalRotation);
		}

		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			localRotation *= current->GetRotation();
		}

		worldRotation = worldRotation_ = localRotation;
		ClearDirty(WorldRotation);
	}

	worldEulerAngles_ = Math::Degrees(glm::eulerAngles(worldRotation));

	ClearDirty(WorldEulerAngles);

	return worldEulerAngles_;
}

void SpriteInternal::SetLocalScale(const glm::vec3& value) {
	ClearDirty(LocalScale);
	if (localScale_ != value) {
		localScale_ = value;
		SetDiry(WorldScale | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::SetLocalPosition(const glm::vec3& value) {
	ClearDirty(LocalPosition);
	if (localPosition_ != value) {
		localPosition_ = value;
		SetDiry(WorldPosition | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::SetLocalRotation(const glm::quat& value) {
	ClearDirty(LocalRotation);
	if (!Math::Approximately(glm::dot(localRotation_, value), 0)) {
		localRotation_ = value;
		SetDiry(WorldRotation | LocalEulerAngles | WorldEulerAngles | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

void SpriteInternal::SetLocalEulerAngles(const glm::vec3& value) {
	ClearDirty(LocalEulerAngles);
	if (localEulerAngles_ == value) {
		localEulerAngles_ = value;
		SetDiry(WorldEulerAngles | LocalRotation | WorldRotation | LocalToWorldMatrix | WorldToLocalMatrix);
	}
}

glm::vec3 SpriteInternal::GetLocalScale() {
	if (IsDirty(LocalScale)) {
		if (IsDirty(WorldScale)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 scale = GetScale();
		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			scale /= current->GetScale();
		}

		localScale_ = scale;
		ClearDirty(LocalScale);
	}

	return localScale_;
}

glm::vec3 SpriteInternal::GetLocalPosition() {
	if (IsDirty(LocalPosition)) {
		if (IsDirty(WorldPosition)) {
			Debug::LogError("invalid state");
		}

		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::vec3 position = GetPosition();
		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			position -= current->GetPosition();
		}

		localPosition_ = position;
		ClearDirty(LocalPosition);
	}

	return localPosition_;
}

glm::quat SpriteInternal::GetLocalRotation() {
	if (!IsDirty(LocalRotation)) { return localRotation_; }

	if (!IsDirty(LocalEulerAngles)) {
		localRotation_ = glm::quat(Math::Radians(localEulerAngles_));
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

			worldRotation = worldRotation_ = glm::quat(Math::Radians(worldEulerAngles_));
			ClearDirty(WorldRotation);
		}

		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			worldRotation *= glm::inverse(current->GetRotation());
		}

		localRotation_ = worldRotation;
	}

	ClearDirty(LocalRotation);

	return localRotation_;
}

glm::vec3 SpriteInternal::GetLocalEulerAngles() {
	if (!IsDirty(LocalEulerAngles)) { return localEulerAngles_; }

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

			worldRotation = worldRotation_ = glm::quat(Math::Radians(worldEulerAngles_));
			ClearDirty(WorldRotation);
		}

		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
			worldRotation *= glm::inverse(current->GetRotation());
		}

		localRotation = localRotation_ = worldRotation;
		ClearDirty(LocalRotation);
	}

	glm::vec3 angles = glm::eulerAngles(localRotation);
	localEulerAngles_ = Math::Degrees(angles);

	ClearDirty(LocalEulerAngles);

	return localEulerAngles_;
}

glm::mat4 SpriteInternal::GetLocalToWorldMatrix() {
	if (IsDirty(LocalToWorldMatrix)) {
		Sprite current = dsp_cast<Sprite>(shared_from_this());
		glm::mat4 matrix = glm::translate(glm::mat4(1), GetLocalPosition()) * glm::mat4(GetLocalRotation()) * glm::scale(glm::mat4(1), GetLocalScale());
		if ((current = current->GetParent()) != worldInstance->GetRootSprite()) {
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
	return localRotation_ * glm::vec3(0, 1, 0);
}

glm::vec3 SpriteInternal::GetRight() {
	return localRotation_ * glm::vec3(1, 0, 0);
}

glm::vec3 SpriteInternal::GetForward() {
	return localRotation_ * glm::vec3(0, 0, -1);
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

Sprite SpriteInternal::FindDirectChild(const std::string& name) {
	for (int i = 0; i < children_.size(); ++i) {
		if (name == children_[i]->GetName()) {
			return children_[i];
		}
	}

	return nullptr;
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
