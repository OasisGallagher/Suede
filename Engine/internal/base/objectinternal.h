#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal : virtual public IObject, public std::enable_shared_from_this<ObjectInternal> {
public:
	ObjectInternal(ObjectType type);
	virtual ~ObjectInternal() {}

public:
	virtual Object Clone();
	virtual ObjectType GetType() { return type_; }
	virtual uint GetInstanceID() { return id_; }

public:
	static void DecodeInstanceID(uint value, ObjectType* type, uint* id);

protected:
	template <class T>
	typename T::Interface SharedThisTraits(T*);

	template <class T, class U>
	T InternalPtrTraits(T, U& obj);

private:
	static uint GenerateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;

	static uint ObjectIDContainer[(int)ObjectType::_Count];
};

template <class T>
typename T::Interface ObjectInternal::SharedThisTraits(T*) {
	return suede_dynamic_cast<T::Interface>(shared_from_this());
}

template <class T, class U>
T ObjectInternal::InternalPtrTraits(T, U& obj) {
	return dynamic_cast<T>(obj.get());
}

#define SharedThis()		SharedThisTraits(this)
#define InternalPtr(obj)	InternalPtrTraits(this, obj)

#define DEFINE_FACTORY_METHOD(name) \
	public: \
		typedef name Interface; \
		static Object Create() { return Factory::Create<name ## Internal>(); } \
	private:
