#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal {
public:
	ObjectInternal(ObjectType type);
	virtual ~ObjectInternal() {}

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

	static uint objectIDContainer[ObjectType::size()];
};

template <class T>
T* IObject::_rptr_impl(T*) const { return (T*)d_; }

template <class T>
typename T::Internal* IObject::_dptr_impl(T*) const { return (T::Internal*)d_; }

template <class T>
std::shared_ptr<T> IObject::_shared_this_impl(T*) { return suede_dynamic_cast<std::shared_ptr<T>>(shared_from_this()); }

// implementation ptr of this.
#define _dptr()			_dptr_impl(this)

// implementation equals.
#define _d_equals(o)	(o).get()->_d_equals_impl(this)

// implementation ptr of o.
#define _rptr(o)		(o).get()->_rptr_impl(this)

// shared ptr of this.
#define _shared_this()	_shared_this_impl(this)
