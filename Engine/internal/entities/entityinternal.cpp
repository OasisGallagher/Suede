#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "tagmanager.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/entityinternal.h"
#include "internal/components/transforminternal.h"

EntityInternal::EntityInternal() : EntityInternal(ObjectTypeEntity) {
}

EntityInternal::EntityInternal(ObjectType entityType)
	: ObjectInternal(entityType), active_(true),  activeSelf_(true), boundsDirty_(true) {
	if (entityType < ObjectTypeEntity || entityType >= ObjectTypeCount) {
		Debug::LogError("invalid entity type %d.", entityType);
	}

	name_ = EntityTypeToString(GetType());
}

void EntityInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(activeSelf_ && transform_->GetParent()->GetEntity()->GetActive());
		UpdateChildrenActive(SharedThis());

		if (renderer_ && mesh_ && !mesh_->GetBounds().IsEmpty()) {
			DirtyParentBounds();
		}
	}
}

bool EntityInternal::SetTag(const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		FireWorldEvent<EntityTagChangedEventPointer>(true);
	}

	return true;
}

void EntityInternal::SetName(const std::string& value) {
	if (value.empty()) {
		Debug::LogWarning("empty name.");
		return;
	}

	if (name_ != value) {
		name_ = value;
		FireWorldEvent<EntityNameChangedEventPointer>(true);
	}
}

void EntityInternal::Update() {
	// TODO: generic component container.
	if (animation_) { animation_->Update(); }
	if (mesh_) { mesh_->Update(); }
	if (particleSystem_) { particleSystem_->Update(); }
	if (renderer_) { renderer_->Update(); }
}

void EntityInternal::SetTransform(Transform value) {
	if (transform_ != value) {
		transform_ = value;
		transform_->SetEntity(SharedThis());
	}
}

void EntityInternal::SetAnimation(Animation value) {
	if (animation_ == value) { return; }

	if (animation_) {
		animation_->SetEntity(nullptr);
	}

	if (animation_ = value) {
		animation_->SetEntity(SharedThis());
	}
}

void EntityInternal::SetMesh(Mesh value) {
	SetComponent(mesh_, value);
	RecalculateBounds();
}

void EntityInternal::SetParticleSystem(ParticleSystem value) {
	SetComponent(particleSystem_, value);
}

void EntityInternal::SetRenderer(Renderer value) {
	if ((!!renderer_) != (!!value)) {
		RecalculateBounds();
	}

	SetComponent(renderer_, value);
}

void EntityInternal::RecalculateBounds() {
	boundsDirty_ = true;
	DirtyParentBounds();
	DirtyChildrenBoundses();
}

void EntityInternal::SetActive(bool value) {
	if (active_ != value) {
		active_ = value;
		FireWorldEvent<EntityActiveChangedEventPointer>(true);
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
		worldBounds_.Clear();
		CalculateHierarchyBounds();
	}

	return worldBounds_;
}

void EntityInternal::CalculateHierarchyBounds() {
	if (animation_) {
		CalculateBonesWorldBounds();
	}
	else {
		CalculateHierarchyMeshBounds();
		boundsDirty_ = false;
	}

	if (particleSystem_) {
		worldBounds_.Encapsulate(particleSystem_->GetMaxBounds());
		boundsDirty_ = true;
	}
}

void EntityInternal::CalculateHierarchyMeshBounds() {
	if (renderer_ && mesh_ && !mesh_->GetBounds().IsEmpty()) {
		CalculateSelfWorldBounds();
	}

	for (uint i = 0; i < transform_->GetChildCount(); ++i) {
		Entity child = transform_->GetChildAt(i)->GetEntity();
		if (child->GetActive()) {
			const Bounds& b = child->GetBounds();
			worldBounds_.Encapsulate(b);
		}
	}
}

void EntityInternal::CalculateSelfWorldBounds() {
	std::vector<glm::vec3> points;
	const Bounds& localBounds = mesh_->GetBounds();
	GeometryUtility::GetCuboidCoordinates(points, localBounds.center, localBounds.size);

	Transform transform = GetTransform();
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < points.size(); ++i) {
		points[i] = transform->TransformPoint(points[i]);

		min = glm::min(min, points[i]);
		max = glm::max(max, points[i]);
	}

	worldBounds_.SetMinMax(min, max);
}

void EntityInternal::CalculateBonesWorldBounds() {
	std::vector<glm::vec3> points;
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	
	Bounds boneBounds;
	Skeleton skeleton = animation_->GetSkeleton();
	glm::mat4* matrices = skeleton->GetBoneToRootMatrices();

	for (uint i = 0; i < skeleton->GetBoneCount(); ++i) {
		SkeletonBone* bone = skeleton->GetBone(i);
		GeometryUtility::GetCuboidCoordinates(points, bone->bounds.center, bone->bounds.size);
		for (uint j = 0; j < points.size(); ++j) {
			points[j] = transform_->TransformPoint(glm::vec3(matrices[i] * glm::vec4(points[j], 1)));

			min = glm::min(min, points[j]);
			max = glm::max(max, points[j]);
		}

		boneBounds.SetMinMax(min, max);
		worldBounds_.Encapsulate(boneBounds);
	}
}

void EntityInternal::DirtyParentBounds() {
	Transform parent, current = transform_;
	for (; (parent = current->GetParent()) && parent != WorldInstance()->GetRootTransform();) {
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
