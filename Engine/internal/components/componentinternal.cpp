#include "componentinternal.h"
#include "componentinternal.h"

void ComponentInternal::SetEntity(Entity entity) {
	if (entity_.lock()) {
		Debug::LogError("component could not be shared.");
		return;
	}

	entity_ = entity;
}
