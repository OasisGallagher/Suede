#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal {
public:
	ObjectInternal(IObject* self, ObjectType type);
	virtual ~ObjectInternal() {}

public:
	virtual Object Clone();

public:
	std::string GetName() const { return name_; }
	void SetName(const std::string& value);

	ObjectType GetObjectType() { return type_; }
	uint GetInstanceID() { return id_; }

protected:
	virtual void OnNameChanged() {}

public:
	static void DecodeInstanceID(uint value, ObjectType* type, uint* id);

protected:
	IObject* self_;

private:
	static uint GenerateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;
	std::string name_;

	static uint objectIDContainer[ObjectType::size()];
};

#define _suede_self()	((Self*)self_)
#define _suede_rptr(o)	((__suede_dtype(this)*)(o).get()->d_)

#define SUEDE_DECLARE_SELF_TYPE(T)	typedef T Self;
