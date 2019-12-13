#include "behaviour.h"

#include "debug/debug.h"
#include "componentinternal.h"

SUEDE_DEFINE_COMPONENT_INTERNAL(Behaviour, Component)

class BehaviourInternal : public ComponentInternal {
public:
	BehaviourInternal() : ComponentInternal(ObjectType::CustomBehaviour) {}

public:
	virtual int GetUpdateStrategy() { return UpdateStrategyRendering; }
};

Behaviour::Behaviour() : Component(new BehaviourInternal) {
}

void Behaviour::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {
	Debug::LogError("Behaviour::OnRenderImage not implement");
}
