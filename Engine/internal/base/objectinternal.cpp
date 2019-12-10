#include "objectinternal.h"

#include "math/mathf.h"
#include "debug/debug.h"

#include "world.h"
#include "textureinternal.h"

IObject::IObject(void* d) : PimplIdiom(d, Memory::DeleteRaw<ObjectInternal>) {}
std::string IObject::GetName() const { return _suede_dptr()->GetName(); }
void IObject::SetName(const std::string& value) { _suede_dptr()->SetName(this, value); }
Object IObject::Clone() { return _suede_dptr()->Clone(); }
ObjectType IObject::GetObjectType() { return _suede_dptr()->GetObjectType(); }
uint IObject::GetInstanceID() { return _suede_dptr()->GetInstanceID(); }

uint ObjectInternal::objectIDContainer[ObjectType::size()];

ObjectInternal::ObjectInternal(ObjectType type) {
	id_ = GenerateInstanceID(type);
	type_ = type;
}

void ObjectInternal::SetName(IObject* self, const std::string& value) {
	if (value.empty()) {
		Debug::LogWarning("empty name.");
		return;
	}

	if (name_ != value) {
		name_ = value;
		OnNameChanged(self);
	}
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

	return Mathf::MakeDword(++objectIDContainer[(int)type], (int)type);
}

void ObjectInternal::DecodeInstanceID(uint value, ObjectType* type, uint* id) {
	if (type != nullptr) { *type = (ObjectType)Mathf::Highword(value); }
	if (id != nullptr) { *id = Mathf::Loword(value); }
}
