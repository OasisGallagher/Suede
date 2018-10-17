#include "objectinternal.h"

#include "tools/math2.h"
#include "debug/debug.h"

#include "textureinternal.h"

IObject::IObject(void* d) : d_(d) {}
IObject::~IObject() { MEMORY_DELETE(_dptr()); }
Object IObject::Clone() { return _dptr()->Clone(); }
ObjectType IObject::GetObjectType() { return _dptr()->GetObjectType(); }
uint IObject::GetInstanceID() { return _dptr()->GetInstanceID(); }\

uint ObjectInternal::ObjectIDContainer[ObjectType::size()];

ObjectInternal::ObjectInternal(ObjectType type) {
	id_ = GenerateInstanceID(type);
	type_ = type;
}

Object ObjectInternal::Clone() {
	Debug::LogError("unable to clone.");
	return nullptr;
}

uint ObjectInternal::GenerateInstanceID(ObjectType type) {
	if (ObjectIDContainer[(int)type] >= std::numeric_limits<uint>::max()) {
		Debug::LogError("too many objects with type %d.", type);
		return 0;
	}

	return Math::MakeDword(++ObjectIDContainer[(int)type], (int)type);
}

void ObjectInternal::DecodeInstanceID(uint value, ObjectType* type, uint* id) {
	if (type != nullptr) { *type = (ObjectType)Math::Highword(value); }
	if (id != nullptr) { *id = Math::Loword(value); }
}
