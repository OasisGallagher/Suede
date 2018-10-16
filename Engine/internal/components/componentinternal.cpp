#include "componentinternal.h"

IComponent::IComponent(void* d) : IObject(d) {}

void IComponent::Awake() {}
void IComponent::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {}
bool IComponent::GetEnabled() const { return dptr()->GetEnabled(); }
void IComponent::SetEnabled(bool value) { dptr()->SetEnabled(value); }
void IComponent::SetGameObject(GameObject go) { dptr()->SetGameObject(go); }
GameObject IComponent::GetGameObject() { return dptr()->GetGameObject(); }
Transform IComponent::GetTransform() { return dptr()->GetTransform(); }
void IComponent::CullingUpdate() { dptr()->CullingUpdate(); }
void IComponent::RenderingUpdate() { dptr()->RenderingUpdate(); }
int IComponent::GetUpdateStrategy() { return dptr()->GetUpdateStrategy(); }

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
