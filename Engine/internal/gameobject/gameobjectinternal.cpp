#include <algorithm>

#include "time2.h"
#include "renderer.h"
#include "rigidbody.h"
#include "tagmanager.h"
#include "math/mathf.h"
#include "tools/string.h"
#include "particlesystem.h"
#include "geometryutility.h"
#include "internal/memory/factory.h"
#include "internal/world/worldinternal.h"
#include "internal/gameobject/gameobjectinternal.h"

GameObject::GameObject(const char* name) : Object(new GameObjectInternal(name)) {
	created.raise(this);
	AddComponent<Transform>();
}

main_mt_event<ref_ptr<GameObject>> GameObject::created;
main_mt_event<ref_ptr<GameObject>> GameObject::destroyed;
main_mt_event<ref_ptr<GameObject>> GameObject::tagChanged;
main_mt_event<ref_ptr<GameObject>> GameObject::nameChanged;
main_mt_event<ref_ptr<GameObject>> GameObject::parentChanged;
main_mt_event<ref_ptr<GameObject>> GameObject::activeChanged;
main_mt_event<ref_ptr<GameObject>, int> GameObject::transformChanged;
main_mt_event<ref_ptr<GameObject>> GameObject::updateStrategyChanged;
main_mt_event<ref_ptr<GameObject>, ComponentEventType, ref_ptr<Component>> GameObject::componentChanged;

bool GameObject::GetActive() const { return _suede_dptr()->GetActive(); }
void GameObject::SetActiveSelf(bool value) { _suede_dptr()->SetActiveSelf(this, value); }
bool GameObject::GetActiveSelf() const { return _suede_dptr()->GetActiveSelf(); }
int GameObject::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(this); }
void GameObject::SendMessage(int messageID, void* parameter) { _suede_dptr()->SendMessage(messageID, parameter); }
const std::string& GameObject::GetTag() const { return _suede_dptr()->GetTag(); }
bool GameObject::SetTag(const std::string& value) { return _suede_dptr()->SetTag(this, value); }
void GameObject::Update() { _suede_dptr()->Update(); }
void GameObject::CullingUpdate() { _suede_dptr()->CullingUpdate(); }
Transform* GameObject::GetTransform() { return _suede_dptr()->GetTransform(); }
const Bounds& GameObject::GetBounds() { return _suede_dptr()->GetBounds(); }
void GameObject::RecalculateBounds(int flags) { return _suede_dptr()->RecalculateBounds(); }
void GameObject::RecalculateUpdateStrategy() { _suede_dptr()->RecalculateUpdateStrategy(this); }
Component* GameObject::AddComponent(suede_guid guid) { return _suede_dptr()->AddComponent(this, guid); }
Component* GameObject::AddComponent(const char* name) { return _suede_dptr()->AddComponent(this, name); }
Component* GameObject::AddComponent(Component* component) { return _suede_dptr()->AddComponent(this, component); }
Component* GameObject::GetComponent(suede_guid guid) { return _suede_dptr()->GetComponent(guid); }
Component* GameObject::GetComponent(const char* name) { return _suede_dptr()->GetComponent(name); }
std::vector<Component*> GameObject::GetComponents(suede_guid guid) { return _suede_dptr()->GetComponents(guid); }
std::vector<Component*> GameObject::GetComponents(const char* name) { return _suede_dptr()->GetComponents(name); }

GameObjectInternal::GameObjectInternal(const char* name)
	: ObjectInternal(ObjectType::GameObject, name), active_(true), activeSelf_(true), boundsDirty_(true)
	, frameCullingUpdate_(0), updateStrategy_(UpdateStrategyNone), updateStrategyDirty_(true) {
}

GameObjectInternal::~GameObjectInternal() {
}

void GameObjectInternal::SetActiveSelf(GameObject* self, bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(self, activeSelf_ && GetTransform()->GetParent()->GetGameObject()->GetActive());
		UpdateChildrenActive(self);

		if (!GetBounds().IsEmpty()) {
			DirtyParentBounds();
		}
	}
}

bool GameObjectInternal::SetTag(GameObject* self, const std::string& value) {
	if (!TagManager::IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
		RaiseGameObjectEvent(GameObject::tagChanged, true, self);
	}

	return true;
}

Component* GameObjectInternal::ActivateComponent(GameObject* self, Component* component) {
	component->SetGameObject(self);
	components_.push_back(component);

	component->Awake();

	if (component->IsComponentType(MeshProvider::GetComponentGUID())) {
		RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);

		if (!GetComponent(Rigidbody::GetComponentGUID())) {
			AddComponent(self, Rigidbody::GetComponentGUID());
		}
	}

	if (component->IsComponentType(Renderer::GetComponentGUID())) {
		RecalculateBounds();
	}

	RaiseGameObjectEvent(GameObject::componentChanged, false, self, ComponentEventType::Added, component);

	RecalculateUpdateStrategy(self);

	return component;
}

int GameObjectInternal::GetUpdateStrategy(GameObject* self) {
	return GetHierarchyUpdateStrategy(self);
}

void GameObjectInternal::SendMessage(int messageID, void* parameter) {
	for (ref_ptr<Component>& component : components_) {
		component->OnMessage(messageID, parameter);
	}
}

void GameObjectInternal::CullingUpdate() {
	uint frame = Time::GetFrameCount();
	if (frameCullingUpdate_ < frame) {
		frameCullingUpdate_ = frame;

		for (ref_ptr<Component>& component : components_) {
			component->CullingUpdate();
		}
	}
}

void GameObjectInternal::Update() {
	for (ref_ptr<Component>& component : components_) {
		component->Update();
	}
}

Transform* GameObjectInternal::GetTransform() {
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

void GameObjectInternal::RecalculateUpdateStrategy(GameObject* self) {
	RecalculateHierarchyUpdateStrategy(self);
}

void GameObjectInternal::OnNameChanged(Object* self) {
	RaiseGameObjectEvent(GameObject::nameChanged, true, (GameObject*)self);
}

void GameObjectInternal::SetActive(GameObject* self, bool value) {
	if (active_ != value) {
		active_ = value;
		RaiseGameObjectEvent(GameObject::activeChanged, true, (GameObject*)self);
	}
}

void GameObjectInternal::UpdateChildrenActive(GameObject* parent) {
	Transform* pr = parent->GetTransform();
	for (int i = 0; i < pr->GetChildCount(); ++i) {
		Transform* transform = pr->GetChildAt(i);
		GameObject* child = transform->GetGameObject();
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
	if (GetComponent<Animation>() != nullptr) {
		CalculateBonesWorldBounds();
	}
	else {
		CalculateHierarchyMeshBounds();
		boundsDirty_ = false;
	}

	ParticleSystem* ps = GetComponent<ParticleSystem>();
	if (ps) {
		worldBounds_.Encapsulate(ps->GetMaxBounds());
		boundsDirty_ = true;
	}
}

void GameObjectInternal::CalculateHierarchyMeshBounds() {
	Renderer* renderer = GetComponent<Renderer>();
	Rigidbody* rigidbody = GetComponent<Rigidbody>();

	if (renderer && rigidbody && !rigidbody->GetBounds().IsEmpty()) {
		CalculateSelfWorldBounds(rigidbody->GetBounds());
	}

	Transform* tr = GetTransform();
	for (int i = 0; i < tr->GetChildCount(); ++i) {
		GameObject* child = tr->GetChildAt(i)->GetGameObject();
		if (child->GetActive()) {
			const Bounds& b = child->GetBounds();
			worldBounds_.Encapsulate(b);
		}
	}
}

void GameObjectInternal::CalculateSelfWorldBounds(const Bounds& bounds) {
	std::vector<Vector3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	Transform* transform = GetTransform();
	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < points.size(); ++i) {
		min = Vector3::Min(min, points[i]);
		max = Vector3::Max(max, points[i]);
	}

	worldBounds_.SetMinMax(min, max);
}

void GameObjectInternal::CalculateBonesWorldBounds() {
	std::vector<Vector3> points;
	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	Bounds boneBounds;
	Skeleton* skeleton = GetComponent<Animation>()->GetSkeleton();
	Matrix4* matrices = skeleton->GetBoneToRootMatrices();

	for (uint i = 0; i < skeleton->GetBoneCount(); ++i) {
		SkeletonBone* bone = skeleton->GetBone(i);
		GeometryUtility::GetCuboidCoordinates(points, bone->bounds.center, bone->bounds.size);
		for (uint j = 0; j < points.size(); ++j) {
			Vector4 p = matrices[i] * Vector4(points[j].x, points[j].y, points[j].z, 1);
			points[j] = GetTransform()->TransformPoint(Vector3(p.x, p.y, p.z));

			min = Vector3::Min(min, points[j]);
			max = Vector3::Max(max, points[j]);
		}

		boneBounds.SetMinMax(min, max);
		worldBounds_.Encapsulate(boneBounds);
	}
}

void GameObjectInternal::DirtyParentBounds() {
	Transform* parent, *current = GetTransform();
	for (; (parent = current->GetParent()) && parent != World::GetRootTransform();) {
		_suede_rptr(parent->GetGameObject())->boundsDirty_ = true;
		current = parent;
	}
}

int GameObjectInternal::GetHierarchyUpdateStrategy(GameObject* root) {
	if (!updateStrategyDirty_) { return updateStrategy_; }

	int strategy = 0;
	for (ref_ptr<Component>& component : components_) {
		strategy |= component->GetUpdateStrategy();
	}

	//for (Transform* tr : root->GetTransform()->GetChildren()) {
	//	strategy |= GetHierarchyUpdateStrategy(tr->GetGameObject());
	//}

	updateStrategy_ = strategy;
	updateStrategyDirty_ = false;

	return strategy;
}

bool GameObjectInternal::RecalculateHierarchyUpdateStrategy(GameObject* self) {
	updateStrategyDirty_ = true;
	int oldStrategy = updateStrategy_;
	int newStrategy = GetUpdateStrategy(self);

	if (oldStrategy != newStrategy) {
		// 		Transform* parent, current = GetTransform();
		// 		for (; (parent = current->GetParent()) && parent != World::GetRootTransform();) {
		// 			if (!_suede_ref_rptr(parent->GetGameObject())->RecalculateHierarchyUpdateStrategy(self)) {
		// 				break;
		// 			}
		// 
		// 			current = parent;
		// 		}

		RaiseGameObjectEvent(GameObject::updateStrategyChanged, false, self);

		return true;
	}

	return false;
}

void GameObjectInternal::DirtyChildrenBoundses() {
	Transform* tr = GetTransform();
	for (int i = 0; i < tr->GetChildCount(); ++i) {
		GameObjectInternal* child = _suede_rptr(tr->GetChildAt(i)->GetGameObject());
		child->DirtyChildrenBoundses();
		child->boundsDirty_ = true;
	}
}
