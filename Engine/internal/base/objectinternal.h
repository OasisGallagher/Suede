#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal : virtual public IObject {
public:
	ObjectInternal(ObjectType type);
	virtual ~ObjectInternal() {}

public:
	virtual ObjectType GetType() { return type_; }
	virtual unsigned GetInstanceID() { return id_; }

public:
	static void DecodeInstanceID(unsigned value, ObjectType* type, unsigned* id);

private:
	static unsigned EncodeInstanceID(ObjectType type);

private:
	unsigned id_;
	ObjectType type_;

	static unsigned ObjectIDContainer[ObjectTypeCount];
};

#define DEFINE_FACTORY_METHOD(name) \
	public: \
		typedef name Interface; \
		static Object Create() { return Factory::Create<name ## Internal>(); } \
	private:
