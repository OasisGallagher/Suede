#pragma once

#include "object.h"
#include "internal/memory/factory.h"

class ObjectInternal {
public:
	ObjectInternal(ObjectType type);
	virtual ~ObjectInternal() {}

public:
	virtual Object Clone();

public:
	std::string GetName() const { return name_; }
	void SetName(IObject* self, const std::string& value);

	ObjectType GetObjectType() { return type_; }
	uint GetInstanceID() { return id_; }

protected:
	virtual void OnNameChanged(IObject* self) {}

public:
	static void DecodeInstanceID(uint value, ObjectType* type, uint* id);

private:
	static uint GenerateInstanceID(ObjectType type);

private:
	uint id_;
	ObjectType type_;
	std::string name_;

	static uint objectIDContainer[ObjectType::size()];
};
