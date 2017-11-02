#pragma once

#include <map>
#include <vector>

#include "object.h"
#include "memory.h"
#include "tools/debug.h"

class Factory {
	typedef Object (*FactoryMethod)();
	typedef std::map<std::string, FactoryMethod> MapContainer;

	Factory();

public:
	template <class Internal>
	static typename Internal::Interface Create() {
		return typename Internal::Interface(Memory::Create<Internal>());
	}

	static Object Create(const std::string& name) {
		MapContainer::iterator pos = instance.methodDictionary_.find(name);
		AssertX(pos != instance.methodDictionary_.end(), "no factroy method exists for: " + name);
		return pos->second();
	}

	static Object Create(ObjectType type) {
		AssertX(type >= 0 && type < ObjectTypeCount, "invalid object type " + std::to_string(type));
		AssertX(instance.methodArray_[type] != nullptr, "no factroy method exists for type: " + std::to_string(type));
		return instance.methodArray_[type]();
	}

private:
	static void AddFactoryMethod(const std::string& name, FactoryMethod method) {
		bool done = instance.methodDictionary_.insert(std::make_pair(name, method)).second;
		AssertX(done, "failed to add factroy method for: " + name);
	}

	static void AddFactoryMethod(ObjectType type, FactoryMethod method) {
		AssertX(instance.methodArray_[type] == nullptr, "method for type " + std::to_string(type) + " already exists.");
		instance.methodArray_[type] = method;
	}

	static Factory instance;
	MapContainer methodDictionary_;
	FactoryMethod methodArray_[ObjectTypeCount];
};

#define CREATE_OBJECT(type)	Factory::Create<type ## Internal>()
