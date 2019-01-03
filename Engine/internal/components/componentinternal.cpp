#include "componentinternal.h"

IComponent::IComponent(void* d) : IObject(d) {}

void IComponent::Awake() { _suede_dptr()->Awake(); }
void IComponent::Update() { _suede_dptr()->Update(); }
void IComponent::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {}

bool IComponent::GetEnabled() const { return _suede_dptr()->GetEnabled(); }
void IComponent::SetEnabled(bool value) { _suede_dptr()->SetEnabled(value); }
void IComponent::SetGameObject(GameObject go) { _suede_dptr()->SetGameObject(go); }
GameObject IComponent::GetGameObject() { return _suede_dptr()->GetGameObject(); }
Transform IComponent::GetTransform() { return _suede_dptr()->GetTransform(); }
void IComponent::OnMessage(int messageID, void* parameter) { _suede_dptr()->OnMessage(messageID, parameter); }
void IComponent::CullingUpdate() { _suede_dptr()->CullingUpdate(); }
int IComponent::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(); }

bool ComponentUtility::Register(suede_guid guid, const std::function<Object()>& creater) {
	return Factory::AddFactoryMethod(guid, creater);
}

bool ComponentUtility::Register(const char* name, const std::function<Object()>& creater) {
	return Factory::AddFactoryMethod(name, creater);
}

suede_guid IComponent::ClassNameToGUID(const char* className) {
	static suede_guid id = 0;
	return ++id;
}

void ComponentInternal::SetGameObject(GameObject go) {
	gameObject_ = go.get();
}
