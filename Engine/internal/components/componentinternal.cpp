#include "componentinternal.h"

IComponent::IComponent(void* d) : IObject(d) {}

void IComponent::Awake() { _suede_dptr()->Awake(); }
void IComponent::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {}
bool IComponent::GetEnabled() const { return _suede_dptr()->GetEnabled(); }
void IComponent::SetEnabled(bool value) { _suede_dptr()->SetEnabled(value); }
void IComponent::SetGameObject(GameObject go) { _suede_dptr()->SetGameObject(go); }
GameObject IComponent::GetGameObject() { return _suede_dptr()->GetGameObject(); }
Transform IComponent::GetTransform() { return _suede_dptr()->GetTransform(); }
void IComponent::CullingUpdate() { _suede_dptr()->CullingUpdate(); }
void IComponent::RenderingUpdate() { _suede_dptr()->RenderingUpdate(); }
int IComponent::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(); }

bool Component::Register(suede_guid guid, Object(*method)()) {
	return Factory::RegisterComponent(guid, method);
}

bool Component::Register(const char* name, Object(*method)()) {
	return Factory::RegisterComponent(name, method);
}

suede_guid IComponent::ClassNameToGUID(const char* className) {
	static suede_guid id = 0;
	return ++id;
}

void ComponentInternal::SetGameObject(GameObject go) {
	if (gameObject_.lock()) {
		Debug::LogError("component could not be shared.");
		return;
	}

	gameObject_ = go;
}
