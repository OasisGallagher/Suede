#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
#include "renderer.h"
#include "rigidbody.h"
#include "tagmanager.h"
#include "geometries.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "layermanager.h"
#include "particlesystem.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/gameobject/gameobjectinternal.h"

IGameObject::IGameObject() : IObject(MEMORY_NEW(GameObjectInternal, this)) {
	World::FireEventImmediate(GameObjectCreatedEvent(this));
}

IGameObject::~IGameObject() {}
bool IGameObject::GetActive() const { return _suede_dptr()->GetActive(); }
void IGameObject::SetActiveSelf(bool value) { _suede_dptr()->SetActiveSelf(value); }
bool IGameObject::GetActiveSelf() const { return _suede_dptr()->GetActiveSelf(); }
void IGameObject::SetLayer(uint value) { _suede_dptr()->SetLayer(value); }
uint IGameObject::GetLayer() const { return _suede_dptr()->GetLayer(); }
int IGameObject::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(); }
void IGameObject::SendMessage(int messageID, void* parameter) { _suede_dptr()->SendMessage(messageID, parameter); }
const std::string& IGameObject::GetTag() const { return _suede_dptr()->GetTag(); }
bool IGameObject::SetTag(const std::string& value) { return _suede_dptr()->SetTag(value); }
void IGameObject::Update() { _suede_dptr()->Update(); }
void IGameObject::OnPreRender() { _suede_dptr()->OnPreRender(); }
void IGameObject::OnPostRender() { _suede_dptr()->OnPostRender(); }
void IGameObject::CullingUpdate() { _suede_dptr()->CullingUpdate(); }
Transform IGameObject::GetTransform() { return _suede_dptr()->GetTransform(); }
const Bounds& IGameObject::GetBounds() { return _suede_dptr()->GetBounds(); }
void IGameObject::RecalculateBounds(int flags) { return _suede_dptr()->RecalculateBounds(); }
void IGameObject::RecalculateUpdateStrategy() { _suede_dptr()->RecalculateUpdateStrategy(); }
Component IGameObject::AddComponent(suede_guid guid) { return _suede_dptr()->AddComponent(guid); }
Component IGameObject::AddComponent(const char* name) { return _suede_dptr()->AddComponent(name); }
Component IGameObject::AddComponent(Component component) { return _suede_dptr()->AddComponent(component); }
Component IGameObject::GetComponent(suede_guid guid) { return _suede_dptr()->GetComponent(guid); }
Component IGameObject::GetComponent(const char* name) { return _suede_dptr()->GetComponent(name); }
std::vector<Component> IGameObject::GetComponents(suede_guid guid) { return _suede_dptr()->GetComponents(guid); }
std::vector<Component> IGameObject::GetComponents(const char* name) { return _suede_dptr()->GetComponents(name); }

GameObjectInternal::GameObjectInternal(IGameObject* self) : GameObjectInternal(self, ObjectType::GameObject) {
}

GameObjectInternal::GameObjectInternal(IGameObject* self, ObjectType type)
	: ObjectInternal(self, type), active_(true), activeSelf_(true), boundsDirty_(true)
	, frameCullingUpdate_(0), updateStrategy_(UpdateStrategyNone), updateStrategyDirty_(true) {
	if (type < ObjectType::GameObject || type >= ObjectType::size()) {
		Debug::LogError("invalid go type %d.", type);
	}

	layer_ = LayerManager::Default;
}

GameObjectInternal::~GameObjectInternal() {
}

void GameObjectInternal::SetActiveSelf(bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(activeSelf_ && GetTransform()->GetParent()->GetGameObject()->GetActive());
		UpdateChildrenActive(_suede_self());

		if ((GetLayer() & ~LayerManager::IgnoreRaycast) != 0 && !GetBounds().IsEmpty()) {
			DirtyParentBounds();
		}
	}
}

bool GameObjectInternal::SetTag(const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		FireWorldEvent<GameObjectTagChangedEvent>(true, false);
	}

	return true;
}

Component GameObjectInternal::ActivateComponent(Component component) {
	component->SetGameObject(_suede_self());
	components_.push_back(component);

	component->Awake();

	if (component->IsComponentType(IMeshProvider::GetComponentGUID())) {
		RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);

		if (!GetComponent(IRigidbody::GetComponentGUID())) {
			AddComponent(IRigidbody::GetComponentGUID());
		}
	}

	if (component->IsComponentType(IRenderer::GetComponentGUID())) {
		RecalculateBounds();
	}

	FireWorldEvent<GameObjectComponentChangedEvent>(false, false, GameObjectComponentChangedEvent::ComponentAdded, component);

	RecalculateUpdateStrategy();

	return component;
}

int GameObjectInternal::GetUpdateStrategy() {
	if (!updateStrategyDirty_) { return updateStrategy_; }

	int strategy = 0;
	for (Component component : components_) {
		strategy |= component->GetUpdateStrategy();
	}

	updateStrategy_ = strategy;
	updateStrategyDirty_ = false;

	return strategy;
}

void GameObjectInternal::SendMessage(int messageID, void* parameter) {
	for (Component component : components_) {
		component->OnMessage(messageID, parameter);
	}
}

void GameObjectInternal::CullingUpdate() {
	uint frame = Time::GetFrameCount();
	if (frameCullingUpdate_ < frame) {
		frameCullingUpdate_ = frame;

		for (Component component : components_) {
			component->CullingUpdate();
		}
	}
}

void GameObjectInternal::Update() {
	for (Component component : components_) {
		if (component->GetEnabled()) {
			component->Update();
		}
	}
}

void GameObjectInternal::OnPreRender() {
	for (Component component : components_) {
		if (component->GetEnabled()) {
			component->OnPreRender();
		}
	}
}

void GameObjectInternal::OnPostRender() {
	for (Component component : components_) {
		if (component->GetEnabled()) {
			component->OnPostRender();
		}
	}
}

Transform GameObjectInternal::GetTransform() {
	return GetComponent<Transform>();
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
	updateStrategyDirty_ = true;
	int oldStrategy = updateStrategy_;
	int newStrategy = GetUpdateStrategy();

	if (oldStrategy != newStrategy) {
		FireWorldEvent<GameObjectUpdateStrategyChangedEvent>(false, false);
	}
}

void GameObjectInternal::OnNameChanged() {
	FireWorldEvent<GameObjectNameChangedEvent>(true, false);
}

void GameObjectInternal::SetActive(bool value) {
	if (active_ != value) {
		active_ = value;
		FireWorldEvent<GameObjectActiveChangedEvent>(true, false);
	}
}

void GameObjectInternal::UpdateChildrenActive(GameObject parent) {
	for (Transform transform : parent->GetTransform()->GetChildren()) {
		GameObject child = transform->GetGameObject();
		GameObjectInternal* childPtr = _suede_rptr(child);
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

void GameObjectInternal::CalculateHierarchyBounds() {
	if (GetComponent<Animation>()) {
		CalculateBonesWorldBounds();
	}
	else {
		CalculateHierarchyMeshBounds();
		boundsDirty_ = false;
	}

	ParticleSystem ps = GetComponent<ParticleSystem>();
	if (ps) {
		worldBounds_.Encapsulate(ps->GetMaxBounds());
		boundsDirty_ = true;
	}
}

void GameObjectInternal::CalculateHierarchyMeshBounds() {
	Renderer renderer = GetComponent<Renderer>();
	Rigidbody rigidbody = GetComponent<Rigidbody>();

	if (renderer && rigidbody && !rigidbody->GetBounds().IsEmpty()) {
		CalculateSelfWorldBounds(rigidbody->GetBounds());
	}

	for (Transform tr : GetTransform()->GetChildren()) {
		GameObject child = tr->GetGameObject();
		if (child->GetActive() && (GetLayer() & ~LayerManager::IgnoreRaycast) != 0) {
			const Bounds& b = child->GetBounds();
			worldBounds_.Encapsulate(b);
		}
	}
}

void GameObjectInternal::CalculateSelfWorldBounds(const Bounds& bounds) {
	std::vector<glm::vec3> points;
	std::vector<uint> indexes;
	Geometries::Cuboid(points, indexes, bounds.center, bounds.size);

	Transform transform = GetTransform();
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < points.size(); ++i) {
		min = Math::Min(min, points[i]);
		max = Math::Max(max, points[i]);
	}

	worldBounds_.SetMinMax(min, max);
}

void GameObjectInternal::CalculateBonesWorldBounds() {
	std::vector<glm::vec3> points;
	std::vector<uint> indexes;
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	Bounds boneBounds;
	Skeleton skeleton = GetComponent<Animation>()->GetSkeleton();
	glm::mat4* matrices = skeleton->GetBoneToRootMatrices();

	for (uint i = 0; i < skeleton->GetBoneCount(); ++i) {
		SkeletonBone* bone = skeleton->GetBone(i);
		Geometries::Cuboid(points, indexes, bone->bounds.center, bone->bounds.size);
		for (uint j = 0; j < points.size(); ++j) {
			points[j] = GetTransform()->TransformPoint(glm::vec3(matrices[i] * glm::vec4(points[j], 1)));

			min = Math::Min(min, points[j]);
			max = Math::Max(max, points[j]);
		}

		boneBounds.SetMinMax(min, max);
		worldBounds_.Encapsulate(boneBounds);

		points.clear();
		indexes.clear();
	}
}

void GameObjectInternal::DirtyParentBounds() {
	Transform parent, current = GetTransform();
	for (; (parent = current->GetParent()) && parent != World::GetRootTransform();) {
		_suede_rptr(parent->GetGameObject())->boundsDirty_ = true;
		current = parent;
	}
}

void GameObjectInternal::DirtyChildrenBoundses() {
	for (Transform tr : GetTransform()->GetChildren()) {
		GameObjectInternal* child = _suede_rptr(tr->GetGameObject());
		child->DirtyChildrenBoundses();
		child->boundsDirty_ = true;
	}
}
