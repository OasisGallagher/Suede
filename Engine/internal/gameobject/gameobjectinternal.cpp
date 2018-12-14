#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
#include "rigidbody.h"
#include "tagmanager.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "geometryutility.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/gameobject/gameobjectinternal.h"

IGameObject::IGameObject() : IObject(MEMORY_NEW(GameObjectInternal)) {}
bool IGameObject::GetActive() const { return _suede_dptr()->GetActive(); }
void IGameObject::SetActiveSelf(bool value) { _suede_dptr()->SetActiveSelf(_shared_this(), value); }
bool IGameObject::GetActiveSelf() const { return _suede_dptr()->GetActiveSelf(); }
int IGameObject::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(_shared_this()); }
void IGameObject::SendMessage(int messageID, void* parameter) { _suede_dptr()->SendMessage(messageID, parameter); }
const std::string& IGameObject::GetTag() const { return _suede_dptr()->GetTag(); }
bool IGameObject::SetTag(const std::string& value) { return _suede_dptr()->SetTag(_shared_this(), value); }
void IGameObject::Update() { _suede_dptr()->Update(); }
void IGameObject::CullingUpdate() { _suede_dptr()->CullingUpdate(); }
Transform IGameObject::GetTransform() { return _suede_dptr()->GetTransform(); }
const Bounds& IGameObject::GetBounds() { return _suede_dptr()->GetBounds(); }
void IGameObject::RecalculateBounds(int flags) { return _suede_dptr()->RecalculateBounds(); }
void IGameObject::RecalculateUpdateStrategy() { _suede_dptr()->RecalculateUpdateStrategy(_shared_this()); }
Component IGameObject::AddComponent(suede_guid guid) { return _suede_dptr()->AddComponent(_shared_this(), guid); }
Component IGameObject::AddComponent(const char* name) { return _suede_dptr()->AddComponent(_shared_this(), name); }
Component IGameObject::AddComponent(Component component) { return _suede_dptr()->AddComponent(_shared_this(), component); }
Component IGameObject::GetComponent(suede_guid guid) { return _suede_dptr()->GetComponent(guid); }
Component IGameObject::GetComponent(const char* name) { return _suede_dptr()->GetComponent(name); }
std::vector<Component> IGameObject::GetComponents(suede_guid guid) { return _suede_dptr()->GetComponents(guid); }
std::vector<Component> IGameObject::GetComponents(const char* name) { return _suede_dptr()->GetComponents(name); }

GameObjectInternal::GameObjectInternal() : GameObjectInternal(ObjectType::GameObject) {
}

GameObjectInternal::GameObjectInternal(ObjectType type)
	: ObjectInternal(type), active_(true), activeSelf_(true), boundsDirty_(true)
	, frameCullingUpdate_(0), updateStrategy_(UpdateStrategyNone), updateStrategyDirty_(true) {
	if (type < ObjectType::GameObject || type >= ObjectType::size()) {
		Debug::LogError("invalid go type %d.", type);
	}
}

GameObjectInternal::~GameObjectInternal() {
}

void GameObjectInternal::SetActiveSelf(GameObject self, bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(self, activeSelf_ && GetTransform()->GetParent()->GetGameObject()->GetActive());
		UpdateChildrenActive(self);

		Renderer renderer = GetComponent<IRenderer>();
		MeshProvider provider = GetComponent<IMeshProvider>();

		if (renderer && provider && !provider->GetMesh()->GetBounds().IsEmpty()) {
			DirtyParentBounds();
		}
	}
}

bool GameObjectInternal::SetTag(GameObject self, const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		FireWorldEvent<GameObjectTagChangedEventPtr>(self, true);
	}

	return true;
}

Component GameObjectInternal::ActivateComponent(GameObject self, Component component) {
	component->SetGameObject(self);
	components_.push_back(component);

	component->Awake();

	if (component->IsComponentType(IMeshFilter::GetComponentGUID())) {
		RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);

		if (!GetComponent(IRigidbody::GetComponentGUID())) {
			AddComponent(self, IRigidbody::GetComponentGUID());
		}
	}

	if (component->IsComponentType(IRenderer::GetComponentGUID())) {
		RecalculateBounds();
	}

	FireWorldEvent<GameObjectComponentChangedEventPtr>(self, false, false, [=](GameObjectComponentChangedEventPtr& event) {
		event->state = GameObjectComponentChangedEvent::ComponentAdded;
		event->component = component;
	});

	RecalculateUpdateStrategy(self);

	return component;
}

int GameObjectInternal::GetUpdateStrategy(GameObject self) {
	return GetHierarchyUpdateStrategy(self);
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
		component->Update();
	}
}

Transform GameObjectInternal::GetTransform() {
	return GetComponent<ITransform>();
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

void GameObjectInternal::RecalculateUpdateStrategy(GameObject self) {
	RecalculateHierarchyUpdateStrategy(self);
}

void GameObjectInternal::OnNameChanged(Object self) {
	FireWorldEvent<GameObjectNameChangedEventPtr>(suede_dynamic_cast<GameObject>(self), true);
}

void GameObjectInternal::SetActive(GameObject self, bool value) {
	if (active_ != value) {
		active_ = value;
		FireWorldEvent<GameObjectActiveChangedEventPtr>(self, true);
	}
}

void GameObjectInternal::UpdateChildrenActive(GameObject parent) {
	for (Transform transform : parent->GetTransform()->GetChildren()) {
		GameObject child = transform->GetGameObject();
		GameObjectInternal* childPtr = _suede_rptr(child);
		childPtr->SetActive(child, childPtr->activeSelf_ && parent->GetActive());
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
	if (GetComponent<IAnimation>()) {
		CalculateBonesWorldBounds();
	}
	else {
		CalculateHierarchyMeshBounds();
		boundsDirty_ = false;
	}

	ParticleSystem ps = GetComponent<IParticleSystem>();
	if (ps) {
		worldBounds_.Encapsulate(ps->GetMaxBounds());
		boundsDirty_ = true;
	}
}

void GameObjectInternal::CalculateHierarchyMeshBounds() {
	Renderer renderer = GetComponent<IRenderer>();
	MeshProvider provider = GetComponent<IMeshProvider>();

	if (renderer && provider && !provider->GetMesh()->GetBounds().IsEmpty()) {
		CalculateSelfWorldBounds(provider->GetMesh());
	}

	for (Transform tr : GetTransform()->GetChildren()) {
		GameObject child = tr->GetGameObject();
		if (child->GetActive()) {
			const Bounds& b = child->GetBounds();
			worldBounds_.Encapsulate(b);
		}
	}
}

void GameObjectInternal::CalculateSelfWorldBounds(Mesh mesh) {
	std::vector<glm::vec3> points;
	const Bounds& localBounds = mesh->GetBounds();
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
	Skeleton skeleton = GetComponent<IAnimation>()->GetSkeleton();
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
	for (; (parent = current->GetParent()) && parent != World::GetRootTransform();) {
		_suede_rptr(parent->GetGameObject())->boundsDirty_ = true;
		current = parent;
	}
}

int GameObjectInternal::GetHierarchyUpdateStrategy(GameObject root) {
	if (!updateStrategyDirty_) { return updateStrategy_; }

	int strategy = 0;
	for (Component component : components_) {
		strategy |= component->GetUpdateStrategy();
	}

	//for (Transform tr : root->GetTransform()->GetChildren()) {
	//	strategy |= GetHierarchyUpdateStrategy(tr->GetGameObject());
	//}

	updateStrategy_ = strategy;
	updateStrategyDirty_ = false;

	return strategy;
}

bool GameObjectInternal::RecalculateHierarchyUpdateStrategy(GameObject self) {
	updateStrategyDirty_ = true;
	int oldStrategy = updateStrategy_;
	int newStrategy = GetUpdateStrategy(self);

	if (oldStrategy != newStrategy) {
// 		Transform parent, current = GetTransform();
// 		for (; (parent = current->GetParent()) && parent != World::GetRootTransform();) {
// 			if (!_suede_rptr(parent->GetGameObject())->RecalculateHierarchyUpdateStrategy(self)) {
// 				break;
// 			}
// 
// 			current = parent;
// 		}

		FireWorldEvent<GameObjectUpdateStrategyChangedEventPtr>(self, false);

		return true;
	}

	return false;
}

void GameObjectInternal::DirtyChildrenBoundses() {
	for (Transform tr : GetTransform()->GetChildren()) {
		GameObjectInternal* child = _suede_rptr(tr->GetGameObject());
		child->DirtyChildrenBoundses();
		child->boundsDirty_ = true;
	}
}
