#include "objectinternal.h"

#include "tools/math2.h"
#include "debug/debug.h"

#include "world.h"
#include "textureinternal.h"

IObject::IObject(void* d) : PimplIdiom(d, Memory::DeleteRaw<ObjectInternal>) {}
bool IObject::IsDestroyed() const { return _suede_dptr()->IsDestroyed(); }
std::string IObject::GetName() const { return _suede_dptr()->GetName(); }
void IObject::SetName(const std::string& value) { _suede_dptr()->SetName(value); }
Object IObject::Clone() { return _suede_dptr()->Clone(); }
ObjectType IObject::GetObjectType() { return _suede_dptr()->GetObjectType(); }
uint IObject::GetInstanceID() { return _suede_dptr()->GetInstanceID(); }
void IObject::Destroy() { _suede_dptr()->Destroy(); }
void IObject::SetHideFlags(HideFlags value) { _suede_dptr()->SetHideFlags(value); }
HideFlags IObject::GetHideFlags() const { return _suede_dptr()->GetHideFlags(); }

uint ObjectInternal::objectIDContainer[ObjectType::size()];

ObjectInternal::ObjectInternal(IObject* self, ObjectType type) 
	: type_(type), self_(self), destroyed_(false), hideFlags_(HideFlags::None) {
	id_ = GenerateInstanceID(type);
}

void ObjectInternal::SetName(const std::string& value) {
	if (name_ != value) {
		name_ = value;
		OnNameChanged();
	}
}

void ObjectInternal::SetHideFlags(HideFlags value) {
	if (hideFlags_ != value) {
		World::FireEvent(new HideFlagsChangedEvent(_suede_self(), hideFlags_));
		hideFlags_ = value;
	}
}

Object ObjectInternal::Clone() {
	Debug::LogError("unable to clone.");
	return nullptr;
}

void ObjectInternal::Destroy() {
	if (destroyed_) {
		Debug::LogError("object has already been destroyed");
	}

	destroyed_ = true;
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
