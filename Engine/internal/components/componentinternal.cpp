#include "componentinternal.h"

IComponent::IComponent(void* d) : IObject(d) {}

void IComponent::Awake() { _dptr()->Awake(); }
void IComponent::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {}
bool IComponent::GetEnabled() const { return _dptr()->GetEnabled(); }
void IComponent::SetEnabled(bool value) { _dptr()->SetEnabled(value); }
void IComponent::SetGameObject(GameObject go) { _dptr()->SetGameObject(go); }
GameObject IComponent::GetGameObject() { return _dptr()->GetGameObject(); }
Transform IComponent::GetTransform() { return _dptr()->GetTransform(); }
void IComponent::CullingUpdate() { _dptr()->CullingUpdate(); }
void IComponent::RenderingUpdate() { _dptr()->RenderingUpdate(); }
int IComponent::GetUpdateStrategy() { return _dptr()->GetUpdateStrategy(); }

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
