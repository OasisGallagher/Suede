#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal : virtual public IObject {
public:
	ObjectInternal(ObjectType type);
	virtual ~ObjectInternal() {}

public:
	virtual Object Clone();
	virtual ObjectType GetType() { return type_; }
	virtual uint GetInstanceID() { return id_; }

public:
	static void DecodeInstanceID(uint value, ObjectType* type, uint* id);

private:
	static uint CreateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;

	static uint ObjectIDContainer[ObjectTypeCount];
};

#define DEFINE_FACTORY_METHOD(name) \
	public: \
		typedef name Interface; \
		static Object Create() { return Factory::Create<name ## Internal>(); } \
	private:
