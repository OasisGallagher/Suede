#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal {
public:
	ObjectInternal(ObjectType type);
	~ObjectInternal() {}

public:
	Object Clone();
	ObjectType GetObjectType() { return type_; }
	uint GetInstanceID() { return id_; }

public:
	static void DecodeInstanceID(uint value, ObjectType* type, uint* id);

private:
	static uint GenerateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;

	static uint ObjectIDContainer[ObjectType::size()];
};

#define dptr()	dptr_impl(this)

#define DEFINE_FACTORY_METHOD(name) \
	public: \
		typedef name Interface; \
		static Object Create() { return Factory::Create<name>(); } \
		static Object Create(size_t type) { return Factory::Create(type); } \
	private:
