#include "componentinternal.h"

#include "internal/base/context.h"

Component::Component(void* d) : Object(d) {}

void Component::Awake() { _suede_dptr()->Awake(); }
void Component::Update(float deltaTime) { _suede_dptr()->Update(deltaTime); }
void Component::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {}

bool Component::GetEnabled() const { return _suede_dptr()->GetEnabled(); }
void Component::SetEnabled(bool value) { _suede_dptr()->SetEnabled(value); }
bool Component::GetActiveAndEnabled() const { return _suede_dptr()->GetActiveAndEnabled(); }
void Component::SetGameObject(GameObject* go) { _suede_dptr()->SetGameObject(go); }
GameObject* Component::GetGameObject() { return _suede_dptr()->GetGameObject(); }
Transform* Component::GetTransform() { return _suede_dptr()->GetTransform(); }
void Component::OnMessage(int messageID, void* parameter) { _suede_dptr()->OnMessage(messageID, parameter); }
void Component::CullingUpdate(float deltaTime) { _suede_dptr()->CullingUpdate(deltaTime); }
int Component::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(); }

bool Component::Register(suede_guid guid, const std::function<Object*()>& creater) { return Factory::AddFactoryMethod(guid, creater); }
bool Component::Register(const char* name, const std::function<Object*()>& creater) { return Factory::AddFactoryMethod(name, creater); }

suede_guid Component::ClassNameToGUID(const char* className) {
	static suede_guid id;
	return ++id;
}

ComponentInternal::ComponentInternal(ObjectType type) : ObjectInternal(type) {
	context_ = (RenderingContext*)Context::GetCurrent();
}

void ComponentInternal::SetGameObject(GameObject* go) {
	gameObject_ = go;
}
