#pragma once

#include "object.h"
#include "internal/memory/factory.h"

#define SUEDE_DECLARE_SELF_TYPE(T)	typedef T Self;

class ObjectInternal {
	SUEDE_DECLARE_SELF_TYPE(IObject)

public:
	ObjectInternal(IObject* self, ObjectType type);
	virtual ~ObjectInternal() {}

public:
	virtual Object Clone();
	virtual void Destroy();

public:
	bool IsDestroyed() const { return destroyed_; }

	std::string GetName() const { return name_; }
	void SetName(const std::string& value);

	ObjectType GetObjectType() { return type_; }
	uint GetInstanceID() { return id_; }

	void SetHideFlags(HideFlags value);
	HideFlags GetHideFlags() const { return hideFlags_; }

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

	bool destroyed_;
	HideFlags hideFlags_;

	static uint objectIDContainer[ObjectType::size()];
};

#define _suede_self()	((Self*)self_)
#define _suede_rptr(o)	((decltype(o)::element_type::Internal*)o->d_)
