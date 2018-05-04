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
	T This();

private:
	static uint GenerateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;

	static uint ObjectIDContainer[ObjectTypeCount];
};

template <class T>
T ObjectInternal::This() {
	return suede_dynamic_cast<T>(shared_from_this());
}

#define DEFINE_FACTORY_METHOD(name) \
	public: \
		typedef name Interface; \
		static Object Create() { return Factory::Create<name ## Internal>(); } \
	private:
