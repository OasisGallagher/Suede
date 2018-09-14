#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
#include "tagmanager.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/entityinternal.h"

#define GET_COMPONENT(T) suede_dynamic_cast<T>(GetComponentHelper(I ## T::GetTypeID()))

EntityInternal::EntityInternal() : EntityInternal(ObjectType::Entity) {
}

EntityInternal::EntityInternal(ObjectType entityType)
	: ObjectInternal(entityType), active_(true),  activeSelf_(true), boundsDirty_(true)
	, frameCullingUpdate_(0), updateStrategy_(UpdateStrategyNone), updateStrategyDirty_(true) {
	if (entityType < ObjectType::Entity || entityType >= ObjectType::size()) {
		Debug::LogError("invalid entity type %d.", entityType);
	}

	name_ = GetObjectType().to_string();
}

EntityInternal::~EntityInternal() {
}

void EntityInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(activeSelf_ && GetTransform()->GetParent()->GetEntity()->GetActive());
		UpdateChildrenActive(SharedThis());

		Renderer renderer = GET_COMPONENT(Renderer);
		MeshFilter meshFilter = GET_COMPONENT(MeshFilter);

		if (renderer && meshFilter && meshFilter->GetMesh() && !meshFilter->GetMesh()->GetBounds().IsEmpty()) {
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

Component EntityInternal::AddComponentHelper(Component component) {
	component->SetEntity(SharedThis());
	components_.push_back(component);

	if (component->IsClassType(IMeshFilter::GetTypeID())) {
		RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
	}

	if (component->IsClassType(IRenderer::GetTypeID())) {
		RecalculateBounds();
	}

	return component;
}

Component EntityInternal::AddComponentHelper(suede_typeid type) {
	if (!CheckComponentDuplicate(type)) {
		return nullptr;
	}

	return AddComponentHelper(suede_dynamic_cast<Component>(Factory::Create(type)));
}

bool EntityInternal::CheckComponentDuplicate(suede_typeid type) {
	if (GetComponentHelper(type)) {
		Debug::LogError("component with type %zu already exist", type);
		return false;
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

		for (Component component : components_) {
			component->CullingUpdate();
		}
	}
}

void EntityInternal::RenderingUpdate() {
	for (Component component : components_) {
		component->RenderingUpdate();
	}
}

Transform EntityInternal::GetTransform() {
	return GET_COMPONENT(Transform);
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

Component EntityInternal::GetComponentHelper(suede_typeid type) {
	for (Component component : components_) {
		if (component->IsClassType(type)) {
			return component;
		}
	}

	return nullptr;
}

std::vector<Component> EntityInternal::GetComponentsHelper(suede_typeid type) {
	std::vector<Component> container;
	for (Component component : components_) {
		if (component->IsClassType(type)) {
			container.push_back(component);
		}
	}

	return container;
}

void EntityInternal::CalculateHierarchyBounds() {
	if (GET_COMPONENT(Animation)) {
		CalculateBonesWorldBounds();
	}
	else {
		CalculateHierarchyMeshBounds();
		boundsDirty_ = false;
	}

	ParticleSystem ps = GET_COMPONENT(ParticleSystem);
	if (ps) {
		worldBounds_.Encapsulate(ps->GetMaxBounds());
		boundsDirty_ = true;
	}
}

void EntityInternal::CalculateHierarchyMeshBounds() {
	Renderer renderer = GET_COMPONENT(Renderer);
	MeshFilter meshFilter = GET_COMPONENT(MeshFilter);

	if (renderer && meshFilter && meshFilter->GetMesh() && !meshFilter->GetMesh()->GetBounds().IsEmpty()) {
		CalculateSelfWorldBounds();
	}

	for (Transform tr : GetTransform()->GetChildren()) {
		Entity child = tr->GetEntity();
		if (child->GetActive()) {
			const Bounds& b = child->GetBounds();
			worldBounds_.Encapsulate(b);
		}
	}
}

void EntityInternal::CalculateSelfWorldBounds() {
	std::vector<glm::vec3> points;
	const Bounds& localBounds = GET_COMPONENT(MeshFilter)->GetMesh()->GetBounds();
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
	Skeleton skeleton = GET_COMPONENT(Animation)->GetSkeleton();
	glm::mat4* matrices = skeleton->GetBoneToRootMatrices();

	for (uint i = 0; i < skeleton->GetBoneCount(); ++i) {
		SkeletonBone* bone = skeleton->GetBone(i);
		GeometryUtility::GetCuboidCoordinates(points, bone->bounds.center, bone->bounds.size);
		for (uint j = 0; j < points.size(); ++j) {
			points[j] = GetTransform()->TransformPoint(glm::vec3(matrices[i] * glm::vec4(points[j], 1)));

			min = glm::min(min, points[j]);
			max = glm::max(max, points[j]);
		}

		boneBounds.SetMinMax(min, max);
		worldBounds_.Encapsulate(boneBounds);
	}
}

void EntityInternal::DirtyParentBounds() {
	Transform parent, current = GetTransform();
	for (; (parent = current->GetParent()) && parent != World::instance()->GetRootTransform();) {
		InternalPtr(parent->GetEntity())->boundsDirty_ = true;
		current = parent;
	}
}

int EntityInternal::GetHierarchyUpdateStrategy(Entity root) {
	if (!updateStrategyDirty_) { return updateStrategy_; }

	int strategy = 0;
	for (Component component : components_) {
		strategy |= component->GetUpdateStrategy();
	}

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
		Transform parent, current = GetTransform();
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
	for (Transform tr : GetTransform()->GetChildren()) {
		EntityInternal* child = InternalPtr(tr->GetEntity());
		child->DirtyChildrenBoundses();
		child->boundsDirty_ = true;
	}
}

const char* EntityInternal::EntityTypeToString(ObjectType type) {
#define CASE(name)	case ObjectType:: ## name: return "ObjectType" #name;
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
