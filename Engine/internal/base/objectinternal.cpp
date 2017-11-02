#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "tools/debug.h"
#include "tools/mathf.h"
#include "objectinternal.h"

unsigned ObjectInternal::ObjectIDContainer[ObjectTypeCount];

ObjectInternal::ObjectInternal(ObjectType type) {
	id_ = EncodeInstanceID(type);
	type_ = type;
}

unsigned ObjectInternal::EncodeInstanceID(ObjectType type) {
	Assert(ObjectIDContainer[type] < std::numeric_limits<unsigned>::max());
	return Mathf::MakeDword(++ObjectIDContainer[type], type);
}

void ObjectInternal::DecodeInstanceID(unsigned value, ObjectType* type, unsigned* id) {
	if (type != nullptr) { *type = (ObjectType)Mathf::Highword(value); }
	if (id != nullptr) { *id = Mathf::Loword(value); }
}
