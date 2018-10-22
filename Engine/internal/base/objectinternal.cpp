#include "objectinternal.h"

#include "tools/math2.h"
#include "debug/debug.h"

#include "textureinternal.h"

IObject::IObject(void* d) : PimplIdiom(d, Memory::DeleteRaw<ObjectInternal>) {}
Object IObject::Clone() { return _suede_dptr()->Clone(); }
ObjectType IObject::GetObjectType() { return _suede_dptr()->GetObjectType(); }
uint IObject::GetInstanceID() { return _suede_dptr()->GetInstanceID(); }

uint ObjectInternal::objectIDContainer[ObjectType::size()];

ObjectInternal::ObjectInternal(ObjectType type) {
	id_ = GenerateInstanceID(type);
	type_ = type;
}

Object ObjectInternal::Clone() {
	Debug::LogError("unable to clone.");
	return nullptr;
}

uint ObjectInternal::GenerateInstanceID(ObjectType type) {
	if (objectIDContainer[(int)type] >= std::numeric_limits<uint>::max()) {
		Debug::LogError("too many objects with type %d.", type);
		return 0;
	}

	return Math::MakeDword(++objectIDContainer[(int)type], (int)type);
}

void ObjectInternal::DecodeInstanceID(uint value, ObjectType* type, uint* id) {
	if (type != nullptr) { *type = (ObjectType)Math::Highword(value); }
	if (id != nullptr) { *id = Math::Loword(value); }
}
