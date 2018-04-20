#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "tagmanager.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/base/transforminternal.h"
#include "internal/entities/entityinternal.h"

EntityInternal::EntityInternal() : EntityInternal(ObjectTypeEntity) {
}

EntityInternal::EntityInternal(ObjectType entityType)
	: ObjectInternal(entityType), active_(true),  activeSelf_(true)
	, boundsDirty_(true) {
	if (entityType < ObjectTypeEntity || entityType >= ObjectTypeCount) {
		Debug::LogError("invalid entity type %d.", entityType);
	}

	name_ = EntityTypeToString(GetType());
}

void EntityInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(activeSelf_ && transform_->GetParent()->GetEntity()->GetActive());
		UpdateChildrenActive(suede_dynamic_cast<Entity>(shared_from_this()));
	}
}

bool EntityInternal::SetTag(const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		EntityTagChangedEventPointer e = NewWorldEvent<EntityTagChangedEventPointer>();
		e->entity = suede_dynamic_cast<Entity>(shared_from_this());
		WorldInstance()->FireEvent(e);
	}

	return true;
}

void EntityInternal::SetName(const std::string& value) {
	if (value.empty()) {
		Debug::LogWarning("empty name.");
	}
	else if (name_ != value) {
		name_ = value;

		EntityNameChangedEventPointer e = NewWorldEvent<EntityNameChangedEventPointer>();
		e->entity = suede_dynamic_cast<Entity>(shared_from_this());
		WorldInstance()->FireEvent(e);
	}
}

void EntityInternal::Update() {
	if (animation_) {
		animation_->Update();
	}
}

void EntityInternal::SetTransform(Transform value) {
	if (transform_ != value) {
		transform_ = value;
		transform_->SetEntity(suede_dynamic_cast<Entity>(shared_from_this()));
	}
}

void EntityInternal::SetInitialBounds(const Bounds& value) {
	initialBounds_ = bounds_ = value;
	RecalculateBounds();
}

void EntityInternal::RecalculateBounds() {
	boundsDirty_ = true;
	DirtyParentBounds();
	DirtyChildrenBoundses();
}

void EntityInternal::SetActive(bool value) {
	if (active_ != value) {
		active_ = value;
		EntityActiveChangedEventPointer e = NewWorldEvent<EntityActiveChangedEventPointer>();
		e->entity = suede_dynamic_cast<Entity>(shared_from_this());
		WorldInstance()->FireEvent(e);
	}
}

void EntityInternal::UpdateChildrenActive(Entity parent) {
	for (int i = 0; i < parent->GetTransform()->GetChildCount(); ++i) {
		Entity child = parent->GetTransform()->GetChildAt(i)->GetEntity();
		EntityInternal* childPtr = dynamic_cast<EntityInternal*>(child.get());
		childPtr->SetActive(childPtr->activeSelf_ && parent->GetActive());
		UpdateChildrenActive(child);
	}
}

const Bounds& EntityInternal::GetBounds() {
	if (boundsDirty_) {
		bounds_.Clear();
		CalculateHierarchyBounds();
	}

	return bounds_;
}

void EntityInternal::CalculateHierarchyBounds() {
	if (!initialBounds_.IsEmpty()) {
		CalculateSelfBounds();
	}

	for (uint i = 0; i < transform_->GetChildCount(); ++i) {
		const Bounds& b = transform_->GetChildAt(i)->GetEntity()->GetBounds();
		bounds_.Encapsulate(b);
	}

	boundsDirty_ = false;
}

void EntityInternal::CalculateSelfBounds() {
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, initialBounds_.center, initialBounds_.size);

	Transform transform = GetTransform();
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < points.size(); ++i) {
		points[i] = transform->TransformPoint(points[i]);

		min = glm::min(min, points[i]);
		max = glm::max(max, points[i]);
	}

	bounds_.SetMinMax(min, max);
}

void EntityInternal::DirtyParentBounds() {
	Transform parent, current = transform_;
	for (; (parent = current->GetParent()) != WorldInstance()->GetRootTransform();) {
		dynamic_cast<EntityInternal*>(parent->GetEntity().get())->boundsDirty_ = true;
		current = parent;
	}
}

void EntityInternal::DirtyChildrenBoundses() {
	for (uint i = 0; i < transform_->GetChildCount(); ++i) {
		EntityInternal* child = dynamic_cast<EntityInternal*>(transform_->GetChildAt(i)->GetEntity().get());
		child->DirtyChildrenBoundses();
		child->boundsDirty_ = true;
	}
}

const char* EntityInternal::EntityTypeToString(ObjectType type) {
#define CASE(name)	case ObjectType ## name: return #name;
	switch (type) {
		CASE(Entity);
		CASE(Camera);
		CASE(Projector);
		CASE(SpotLight);
		CASE(PointLight);
		CASE(DirectionalLight);
		CASE(ParticleSystem);
	}
#undef CASE

	Debug::LogError("entity name for %d does not exist.", type);
	return "";
}
