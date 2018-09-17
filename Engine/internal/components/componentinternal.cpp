#include "componentinternal.h"

suede_guid IComponent::ComponentGUID(const char* classname) {
	static suede_guid id = 0;
	return ++id;
	/*return std::hash<std::string>()(classname);*/
}

void ComponentInternal::SetGameObject(GameObject go) {
	if (gameObject_.lock()) {
		Debug::LogError("component could not be shared.");
		return;
	}

	gameObject_ = go;
}
