#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal {
public:
	ObjectInternal(ObjectType type);
	virtual ~ObjectInternal() {}

public:
	std::string GetName() const { return name_; }
	void SetName(Object* self, const std::string& value);

	ref_ptr<Object> Clone();
	ObjectType GetObjectType() { return type_; }
	uint GetInstanceID() { return id_; }

protected:
	virtual void OnNameChanged(Object* self) {}

public:
	static void DecodeInstanceID(uint value, ObjectType* type, uint* id);

protected:
	std::string name_;

private:
	static uint GenerateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;

	static uint objectIDContainer[ObjectType::size()];
};
