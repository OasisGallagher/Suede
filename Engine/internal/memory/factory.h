#pragma once

#include <map>
#include <vector>

#include "debug.h"
#include "object.h"
#include "memory.h"

class Factory {
	typedef Object (*FactoryMethod)();
	typedef std::map<std::string, FactoryMethod> MapContainer;

	Factory();

public:
	template <class Internal>
	static typename Internal::Interface Create() {
		return typename Internal::Interface(MEMORY_CREATE(Internal), Deleter(), Allocator<Internal>());
	}

	static Object Create(const std::string& name) {
		MapContainer::iterator pos = instance.methodDictionary_.find(name);
		if (pos == instance.methodDictionary_.end()) {
			Debug::LogError("no factroy method exists for %s", name.c_str());
			return nullptr;
		}

		return pos->second();
	}

	static Object Create(ObjectType type) {
		if (type < 0 || type >= ObjectTypeCount) {
			Debug::LogError("invalid object type %d.", type);
			return nullptr;
		}

		if (instance.methodArray_[type] == nullptr) {
			Debug::LogError("no factroy method exists for type %d.", type);
			return nullptr;
		}

		return instance.methodArray_[type]();
	}

private:
	static void AddFactoryMethod(const std::string& name, FactoryMethod method) {
		if (!instance.methodDictionary_.insert(std::make_pair(name, method)).second) {
			Debug::LogError("failed to add factroy method for %s", name.c_str());
		}
	}

	static void AddFactoryMethod(ObjectType type, FactoryMethod method) {
		if (instance.methodArray_[type] != nullptr) {
			Debug::LogError("method for type %d already exists.", type);
			return;
		}

		instance.methodArray_[type] = method;
	}

	static Factory instance;
	MapContainer methodDictionary_;
	FactoryMethod methodArray_[ObjectTypeCount];
};
