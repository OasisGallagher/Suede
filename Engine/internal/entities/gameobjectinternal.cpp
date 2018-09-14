#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
#include "tagmanager.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/gameobjectinternal.h"

#define GET_COMPONENT(T) suede_dynamic_cast<T>(GetComponentHelper(I ## T::GetTypeID()))

GameObjectInternal::GameObjectInternal() : GameObjectInternal(ObjectType::GameObject) {
}

GameObjectInternal::GameObjectInternal(ObjectType gameObjectType)
	: ObjectInternal(gameObjectType), active_(true),  activeSelf_(true), boundsDirty_(true)
	, frameCullingUpdate_(0), updateStrategy_(UpdateStrategyNone), updateStrategyDirty_(true) {
	if (gameObjectType < ObjectType::GameObject || gameObjectType >= ObjectType::size()) {
		Debug::LogError("invalid go type %d.", gameObjectType);
	}

	name_ = GetObjectType().to_string();
}

GameObjectInternal::~GameObjectInternal() {
}

void GameObjectInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(activeSelf_ && GetTransform()->GetParent()->GetGameObject()->GetActive());
		UpdateChildrenActive(SharedThis());

		Renderer renderer = GET_COMPONENT(Renderer);
		MeshFilter meshFilter = GET_COMPONENT(MeshFilter);

		if (renderer && meshFilter && meshFilter->GetMesh() && !meshFilter->GetMesh()->GetBounds().IsEmpty()) {
			DirtyParentBounds();
		}
	}
}

bool GameObjectInternal::SetTag(const std::string& value) {
	if (!TagManager::instance()->IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		FireWorldEvent<GameObjectTagChangedEventPointer>(true);
	}

	return true;
}

Component GameObjectInternal::AddComponentHelper(Component component) {
	component->SetGameObject(SharedThis());
	components_.push_back(component);

	if (component->IsClassType(IMeshFilter::GetTypeID())) {
		RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
	}

	if (component->IsClassType(IRenderer::GetTypeID())) {
		RecalculateBounds();
	}

	auto e = NewWorldEvent<GameObjectComponentChangedEventPointer>();
	e->go = SharedThis();
	e->added = true;
	e->component = component;
	World::instance()->FireEvent(e);

	return component;
}

Component GameObjectInternal::AddComponentHelper(suede_guid type) {
	if (!CheckComponentDuplicate(type)) {
		return nullptr;
	}

	return AddComponentHelper(suede_dynamic_cast<Component>(Factory::Create(type)));
}

bool GameObjectInternal::CheckComponentDuplicate(suede_guid type) {
	if (GetComponentHelper(type)) {
		Debug::LogError("component with type %u already exist", type);
		return false;
	}

	return true;
}

int GameObjectInternal::GetUpdateStrategy() {
	return GetHierarchyUpdateStrategy(SharedThis());
}

void GameObjectInternal::SetName(const std::string& value) {
	if (value.empty()) {
		Debug::LogWarning("empty name.");
		return;
	}

	if (name_ != value) {
		name_ = value;
		FireWorldEvent<GameObjectNameChangedEventPointer>(true);
	}
}

void GameObjectInternal::CullingUpdate() {
	uint frame = Time::instance()->GetFrameCount();
	if (frameCullingUpdate_ < frame) {
		frameCullingUpdate_ = frame;

		for (Component component : components_) {
			component->CullingUpdate();
		}
	}
}

void GameObjectInternal::RenderingUpdate() {
	for (Component component : components_) {
		component->RenderingUpdate();
	}
}

Transform GameObjectInternal::GetTransform() {
	return GET_COMPONENT(Transform);
}

void GameObjectInternal::RecalculateBounds(int flags) {
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

void GameObjectInternal::RecalculateUpdateStrategy() {
	RecalculateHierarchyUpdateStrategy();
}

void GameObjectInternal::SetActive(bool value) {
	if (active_ != value) {
		active_ = value;
		FireWorldEvent<GameObjectActiveChangedEventPointer>(true);
	}
}

void GameObjectInternal::UpdateChildrenActive(GameObject parent) {
	for (Transform transform : parent->GetTransform()->GetChildren()) {
		GameObject child = transform->GetGameObject();
		GameObjectInternal* childPtr = InternalPtr(child);
		childPtr->SetActive(childPtr->activeSelf_ && parent->GetActive());
		UpdateChildrenActive(child);
	}
}

const Bounds& GameObjectInternal::GetBounds() {
	if (boundsDirty_) {
		worldBounds_.Clear();
		CalculateHierarchyBounds();
	}

	return worldBounds_;
}

Component GameObjectInternal::GetComponentHelper(suede_guid type) {
	for (Component component : components_) {
		if (component->IsClassType(type)) {
			return component;
		}
	}

	return nullptr;
}

std::vector<Component> GameObjectInternal::GetComponentsHelper(suede_guid type) {
	std::vector<Component> container;
	for (Component component : components_) {
		if (component->IsClassType(type)) {
			container.push_back(component);
		}
	}

	return container;
}

void GameObjectInternal::CalculateHierarchyBounds() {
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

void GameObjectInternal::CalculateHierarchyMeshBounds() {
	Renderer renderer = GET_COMPONENT(Renderer);
	MeshFilter meshFilter = GET_COMPONENT(MeshFilter);

	if (renderer && meshFilter && meshFilter->GetMesh() && !meshFilter->GetMesh()->GetBounds().IsEmpty()) {
		CalculateSelfWorldBounds();
	}

	for (Transform tr : GetTransform()->GetChildren()) {
		GameObject child = tr->GetGameObject();
		if (child->GetActive()) {
			const Bounds& b = child->GetBounds();
			worldBounds_.Encapsulate(b);
		}
	}
}

void GameObjectInternal::CalculateSelfWorldBounds() {
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

void GameObjectInternal::CalculateBonesWorldBounds() {
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

void GameObjectInternal::DirtyParentBounds() {
	Transform parent, current = GetTransform();
	for (; (parent = current->GetParent()) && parent != World::instance()->GetRootTransform();) {
		InternalPtr(parent->GetGameObject())->boundsDirty_ = true;
		current = parent;
	}
}

int GameObjectInternal::GetHierarchyUpdateStrategy(GameObject root) {
	if (!updateStrategyDirty_) { return updateStrategy_; }

	int strategy = 0;
	for (Component component : components_) {
		strategy |= component->GetUpdateStrategy();
	}

	for (Transform tr : root->GetTransform()->GetChildren()) {
		strategy |= GetHierarchyUpdateStrategy(tr->GetGameObject());
	}

	updateStrategy_ = strategy;
	updateStrategyDirty_ = false;

	return strategy;
}

bool GameObjectInternal::RecalculateHierarchyUpdateStrategy() {
	updateStrategyDirty_ = true;
	int oldStrategy = updateStrategy_;
	int newStrategy = GetUpdateStrategy();

	if (oldStrategy != newStrategy) {
		Transform parent, current = GetTransform();
		for (; (parent = current->GetParent()) && parent != World::instance()->GetRootTransform();) {
			if (!InternalPtr(parent->GetGameObject())->RecalculateHierarchyUpdateStrategy()) {
				break;
			}

			current = parent;
		}

		GameObjectUpdateStrategyChangedEventPointer e = NewWorldEvent<GameObjectUpdateStrategyChangedEventPointer>();
		e->go = SharedThis();
		World::instance()->FireEvent(e);

		return true;
	}

	return false;
}

void GameObjectInternal::DirtyChildrenBoundses() {
	for (Transform tr : GetTransform()->GetChildren()) {
		GameObjectInternal* child = InternalPtr(tr->GetGameObject());
		child->DirtyChildrenBoundses();
		child->boundsDirty_ = true;
	}
}
