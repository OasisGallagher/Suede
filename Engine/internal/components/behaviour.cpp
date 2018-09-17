#include "behaviour.h"

#include "debug/debug.h"
#include "componentinternal.h"

SUEDE_DEFINE_COMPONENT(Behaviour, IComponent)

class BehaviourImpl : public ComponentInternal {
public:
	BehaviourImpl() : ComponentInternal(ObjectType::CustomBehaviour) {}

public:
	virtual int GetUpdateStrategy() { return UpdateStrategyRendering; }
};

Behaviour::Behaviour() {
	impl = MEMORY_NEW(BehaviourImpl);
}

Behaviour::~Behaviour() {
	MEMORY_DELETE(impl);
}

bool Behaviour::GetEnabled() const {
	return impl->GetEnabled();
}

void Behaviour::SetEnabled(bool value) {
	impl->SetEnabled(value);
}

void Behaviour::SetGameObject(GameObject value) {
	impl->SetGameObject(value);
}

GameObject Behaviour::GetGameObject() {
	return impl->GetGameObject();
}

Transform Behaviour::GetTransform() {
	return impl->GetTransform();
}

void Behaviour::CullingUpdate() {
}

void Behaviour::RenderingUpdate() {
	Update();
}

int Behaviour::GetUpdateStrategy() {
	return impl->GetUpdateStrategy();
}

Object Behaviour::Clone() {
	return impl->Clone();
}

ObjectType Behaviour::GetObjectType() {
	return impl->GetObjectType();
}

uint Behaviour::GetInstanceID() {
	return impl->GetInstanceID();
}
