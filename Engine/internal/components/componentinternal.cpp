#include "componentinternal.h"

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
