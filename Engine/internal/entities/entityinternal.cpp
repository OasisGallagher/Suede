#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
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
	: ObjectInternal(entityType), active_(true),  activeSelf_(true), boundsDirty_(true)
	, frameCullingUpdate_(0), updateStrategy_(UpdateStrategyNone), updateStrategyDirty_(true) {
	if (entityType < ObjectTypeEntity || entityType >= ObjectTypeCount) {
		Debug::LogError("invalid entity type %d.", entityType);
	}

	name_ = EntityTypeToString(GetType());
}

EntityInternal::~EntityInternal() {
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
	if (!TagManager::instance()->IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		FireWorldEvent<EntityTagChangedEventPointer>(true);
	}

	return true;
}

int EntityInternal::GetUpdateStrategy() {
	return GetHierarchyUpdateStrategy(SharedThis());
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

void EntityInternal::CullingUpdate() {
	uint frame = Time::instance()->GetFrameCount();
	if (frameCullingUpdate_ < frame) {
		frameCullingUpdate_ = frame;

		if (animation_) { animation_->CullingUpdate(); }
		if (mesh_) { mesh_->CullingUpdate(); }
		if (particleSystem_) { particleSystem_->CullingUpdate(); }
		if (renderer_) { renderer_->CullingUpdate(); }
	}
}

void EntityInternal::RenderingUpdate() {
	if (animation_) { animation_->RenderingUpdate(); }
	if (mesh_) { mesh_->RenderingUpdate(); }
	if (particleSystem_) { particleSystem_->RenderingUpdate(); }
	if (renderer_) { renderer_->RenderingUpdate(); }
}

void EntityInternal::SetTransform(Transform value) {
	SetComponent(transform_, value);
}

void EntityInternal::SetAnimation(Animation value) {
	SetComponent(animation_, value);
}

void EntityInternal::SetMesh(Mesh value) {
	SetComponent(mesh_, value);
	RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
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

void EntityInternal::RecalculateBounds(int flags) {
	if ((flags & RecalculateBoundsFlagsSelf) != 0) {
		boundsDirty_ = true;
	}

	if ((flags & RecalculateBoundsFlagsParent) != 0) {
		DirtyParentBounds();
	}

	if ((flags & RecalculateBoundsFlagsChildren) != 0) {
		DirtyChildrenBoundses();
	}
}

void EntityInternal::RecalculateUpdateStrategy() {
	RecalculateHierarchyUpdateStrategy();
}

void EntityInternal::SetActive(bool value) {
	if (active_ != value) {
		active_ = value;
		FireWorldEvent<EntityActiveChangedEventPointer>(true);
	}
}

void EntityInternal::UpdateChildrenActive(Entity parent) {
	for (Transform transform : parent->GetTransform()->GetChildren()) {
		Entity child = transform->GetEntity();
		EntityInternal* childPtr = InternalPtr(child);
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

	for (Transform tr : transform_->GetChildren()) {
		Entity child = tr->GetEntity();
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
	for (; (parent = current->GetParent()) && parent != World::instance()->GetRootTransform();) {
		InternalPtr(parent->GetEntity())->boundsDirty_ = true;
		current = parent;
	}
}

int EntityInternal::GetHierarchyUpdateStrategy(Entity root) {
	if (!updateStrategyDirty_) { return updateStrategy_; }

	int strategy = 0;
	if (animation_) { strategy |= animation_->GetUpdateStrategy(); }
	if (mesh_) { strategy |= mesh_->GetUpdateStrategy(); }
	if (particleSystem_) { strategy |= particleSystem_->GetUpdateStrategy(); }
	if (renderer_) { strategy |= renderer_->GetUpdateStrategy(); }

	for (Transform tr : root->GetTransform()->GetChildren()) {
		strategy |= GetHierarchyUpdateStrategy(tr->GetEntity());
	}

	updateStrategy_ = strategy;
	updateStrategyDirty_ = false;

	return strategy;
}

bool EntityInternal::RecalculateHierarchyUpdateStrategy() {
	updateStrategyDirty_ = true;
	int oldStrategy = updateStrategy_;
	int newStrategy = GetUpdateStrategy();

	if (oldStrategy != newStrategy) {
		Transform parent, current = transform_;
		for (; (parent = current->GetParent()) && parent != World::instance()->GetRootTransform();) {
			if (!InternalPtr(parent->GetEntity())->RecalculateHierarchyUpdateStrategy()) {
				break;
			}

			current = parent;
		}

		EntityUpdateStrategyChangedEventPointer e = NewWorldEvent<EntityUpdateStrategyChangedEventPointer>();
		e->entity = SharedThis();
		World::instance()->FireEvent(e);

		return true;
	}

	return false;
}

void EntityInternal::DirtyChildrenBoundses() {
	for (Transform tr : transform_->GetChildren()) {
		EntityInternal* child = InternalPtr(tr->GetEntity());
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
