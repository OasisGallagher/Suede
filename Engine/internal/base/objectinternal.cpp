#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "math2.h"
#include "debug.h"
#include "objectinternal.h"

uint ObjectInternal::ObjectIDContainer[ObjectTypeCount];

ObjectInternal::ObjectInternal(ObjectType type) {
	id_ = CreateInstanceID(type);
	type_ = type;
}

Object ObjectInternal::Clone() {
	return Object();
}

uint ObjectInternal::CreateInstanceID(ObjectType type) {
	if (ObjectIDContainer[type] >= std::numeric_limits<uint>::max()) {
		Debug::LogError("too many objects with type %d.", type);
		return 0;
	}

	return Math::MakeDword(++ObjectIDContainer[type], type);
}

void ObjectInternal::DecodeInstanceID(uint value, ObjectType* type, uint* id) {
	if (type != nullptr) { *type = (ObjectType)Math::Highword(value); }
	if (id != nullptr) { *id = Math::Loword(value); }
}
