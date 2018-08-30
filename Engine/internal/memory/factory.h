#pragma once

#include <map>
#include <vector>

#include "object.h"
#include "debug/debug.h"
#include "memory/memory.h"

class Factory {
	typedef Object (*FactoryMethod)();
	typedef std::map<std::string, FactoryMethod> MethodDictionary;

	Factory();

public:
	template <class Internal>
	static typename Internal::Interface Create() {
		return typename Internal::Interface(MEMORY_NEW(Internal), Deleter(), Allocator<Internal>());
	}

	static Object Create(const std::string& name) {
		MethodDictionary::iterator pos = instance.methodDictionary_.find(name);
		if (pos == instance.methodDictionary_.end()) {
			Debug::LogError("no factroy method exists for %s", name.c_str());
			return nullptr;
		}

		return pos->second();
	}

	static Object Create(ObjectType type) {
		if ((int)type < 0 || type >= ObjectType::_Count) {
			Debug::LogError("invalid object type %d.", type);
			return nullptr;
		}

		if (instance.methodArray_[(int)type] == nullptr) {
			Debug::LogError("no factroy method exists for type %d.", type);
			return nullptr;
		}

		return instance.methodArray_[(int)type]();
	}

private:
	static void AddFactoryMethod(const std::string& name, FactoryMethod method) {
		if (!instance.methodDictionary_.insert(std::make_pair(name, method)).second) {
			Debug::LogError("failed to add factroy method for %s", name.c_str());
		}
	}

	static void AddFactoryMethod(ObjectType type, FactoryMethod method) {
		if (instance.methodArray_[(int)type] != nullptr) {
			Debug::LogError("method for type %d already exists.", type);
			return;
		}

		instance.methodArray_[(int)type] = method;
	}

	static Factory instance;
	MethodDictionary methodDictionary_;
	FactoryMethod methodArray_[(int)ObjectType::_Count];
};
