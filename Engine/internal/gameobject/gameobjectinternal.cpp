#include <algorithm>

#include "time2.h"
#include "scene.h"
#include "renderer.h"
#include "rigidbody.h"
#include "tags.h"
#include "math/mathf.h"
#include "tools/string.h"
#include "particlesystem.h"
#include "geometryutility.h"
#include "internal/base/context.h"
#include "internal/memory/factory.h"
#include "internal/engine/engineinternal.h"
#include "internal/gameobject/gameobjectinternal.h"

GameObject::GameObject(const char* name)
	: Object(new GameObjectInternal(Context::GetCurrent(), Engine::GetSubsystem<Scene>(), Engine::GetSubsystem<Tags>(), name)) {
	created.raise(this);
	AddComponent<Transform>();
}

// TODO main thread event...
main_mt_event<ref_ptr<GameObject>> GameObject::created;
main_mt_event<ref_ptr<GameObject>> GameObject::destroyed;
main_mt_event<ref_ptr<GameObject>> GameObject::nameChanged;
main_mt_event<ref_ptr<GameObject>> GameObject::parentChanged;
main_mt_event<ref_ptr<GameObject>> GameObject::activeChanged;

event<ref_ptr<GameObject>, ComponentEventType, ref_ptr<Component>> GameObjectInternal::componentChanged;

bool GameObject::GetActive() const { return _suede_dptr()->GetActive(); }
Scene* GameObject::GetScene() { return _suede_dptr()->GetScene(); }
void GameObject::SetActiveSelf(bool value) { _suede_dptr()->SetActiveSelf(this, value); }
bool GameObject::GetActiveSelf() const { return _suede_dptr()->GetActiveSelf(); }
int GameObject::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(this); }
void GameObject::SendMessage(int messageID, void* parameter) { _suede_dptr()->SendMessage(messageID, parameter); }
const std::string& GameObject::GetTag() const { return _suede_dptr()->GetTag(); }
bool GameObject::SetTag(const std::string& value) { return _suede_dptr()->SetTag(this, value); }
void GameObject::Update(float deltaTime) { _suede_dptr()->Update(deltaTime); }
void GameObject::CullingUpdate(float deltaTime) { _suede_dptr()->CullingUpdate(deltaTime); }
Transform* GameObject::GetTransform() { return _suede_dptr()->GetTransform(); }
void GameObject::RecalculateUpdateStrategy() { _suede_dptr()->RecalculateUpdateStrategy(this); }
Component* GameObject::AddComponent(suede_guid guid) { return _suede_dptr()->AddComponent(this, guid); }
Component* GameObject::AddComponent(const char* name) { return _suede_dptr()->AddComponent(this, name); }
Component* GameObject::AddComponent(Component* component) { return _suede_dptr()->AddComponent(this, component); }
Component* GameObject::GetComponent(suede_guid guid) { return _suede_dptr()->GetComponent(guid); }
Component* GameObject::GetComponent(const char* name) { return _suede_dptr()->GetComponent(name); }
std::vector<Component*> GameObject::GetComponentsInChildren(suede_guid guid) { return _suede_dptr()->GetComponentsInChildren(guid); }
std::vector<Component*> GameObject::GetComponentsInChildren(const char* name) { return _suede_dptr()->GetComponentsInChildren(name); }
std::vector<Component*> GameObject::GetComponents(suede_guid guid) { return _suede_dptr()->GetComponents(guid); }
std::vector<Component*> GameObject::GetComponents(const char* name) { return _suede_dptr()->GetComponents(name); }

GameObjectInternal::GameObjectInternal(Context* context, Scene* scene, Tags* tags, const char* name)
	: ObjectInternal(ObjectType::GameObject, name), context_(context), scene_(scene), tags_(tags) {
}

GameObjectInternal::~GameObjectInternal() {
}

void GameObjectInternal::SetActiveSelf(GameObject* self, bool value) {
	if (activeSelf_ != value) {
		activeSelf_ = value;
		SetActive(self, activeSelf_ && GetTransform()->GetParent()->GetGameObject()->GetActive());
		UpdateChildrenActive(self);
	}
}

bool GameObjectInternal::SetTag(GameObject* self, const std::string& value) {
	if (!tags_->IsRegistered(value)) {
		Debug::LogError("invalid tag \"%s\". please register it first.", value.c_str());
		return false;
	}

	if (tag_ != value) {
		tag_ = value;
	}

	return true;
}

Component* GameObjectInternal::ActivateComponent(GameObject* self, Component* component) {
	component->SetGameObject(self);
	components_.push_back(component);

	component->Awake();

	if (component->IsComponentType(MeshProvider::GetComponentGUID())) {
		if (!GetComponent(Rigidbody::GetComponentGUID())) {
			ActivateComponent(self, new Rigidbody());
		}
	}

	RecalculateUpdateStrategy(self);
	GameObjectInternal::componentChanged.raise(self, ComponentEventType::Added, component);

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

void GameObjectInternal::CullingUpdate(float deltaTime) {
	for (ref_ptr<Component>& component : components_) {
		component->CullingUpdate(deltaTime);
	}
}

void GameObjectInternal::Update(float deltaTime) {
	for (ref_ptr<Component>& component : components_) {
		component->Update(deltaTime);
	}
}

Transform* GameObjectInternal::GetTransform() {
	return GetComponent<Transform>();
}

void GameObjectInternal::RecalculateUpdateStrategy(GameObject* self) {
	RecalculateHierarchyUpdateStrategy(self);
}

void GameObjectInternal::OnNameChanged(Object* self) {
	GameObject::nameChanged.raise((GameObject*)self);
}

void GameObjectInternal::SetActive(GameObject* self, bool value) {
	if (active_ != value) {
		active_ = value;
		GameObject::activeChanged.raise((GameObject*)self);
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
		//Transform* parent, current = GetTransform();
		//for (; (parent = current->GetParent()) && parent != Engine::GetRootTransform();) {
		//	if (!_suede_rptr(parent->GetGameObject().get())->RecalculateHierarchyUpdateStrategy(self)) {
		//		break;
		//	}

		//	current = parent;
		//}

		return true;
	}

	return false;
}
