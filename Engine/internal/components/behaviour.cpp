#include "behaviour.h"

#include "debug/debug.h"
#include "componentinternal.h"

SUEDE_DEFINE_COMPONENT_INTERNAL(Behaviour, Component)

class BehaviourInternal : public ComponentInternal {
	SUEDE_DECLARE_SELF_TYPE(IBehaviour)

public:
	BehaviourInternal(IBehaviour* self) : ComponentInternal(self, ObjectType::CustomBehaviour) {}

public:
	virtual int GetUpdateStrategy() { return UpdateStrategyRendering; }
};

IBehaviour::IBehaviour() : IComponent(MEMORY_NEW(BehaviourInternal, this)) {
}

void IBehaviour::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
	Debug::LogError("Behaviour::OnRenderImage not implement");
}
