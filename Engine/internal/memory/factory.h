#pragma once

#include <map>
#include <vector>

#include "object.h"
#include "debug/debug.h"
#include "memory/memory.h"

class Factory {
	typedef Object (*FactoryMethod)();
	typedef std::map<suede_typeid, FactoryMethod> TypeIDMethodDictionary;
	typedef std::map<std::string, FactoryMethod> NameMethodDictionary;

	Factory();

public:
	template <class Internal>
	static typename Internal::Interface Create() {
		return typename Internal::Interface(MEMORY_NEW(Internal), Deleter(), Allocator<Internal>());
	}

	static Object Create(const std::string& name) {
		NameMethodDictionary::iterator pos = instance.stringMethodDictionary_.find(name);
		if (pos == instance.stringMethodDictionary_.end()) {
			Debug::LogError("no factroy method exists for %s", name.c_str());
			return nullptr;
		}

		return pos->second();
	}

	static Object Create(suede_typeid type) {
		TypeIDMethodDictionary::iterator pos = instance.typeIDMethodDictionary_.find(type);
		if (pos == instance.typeIDMethodDictionary_.end()) {
			Debug::LogError("no factroy method exists for type %zu.", type);
			return nullptr;
		}

		return pos->second();
	}

	static Object Create(ObjectType type) {
		if ((int)type < 0 || type >= ObjectType::size()) {
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
		if (!instance.stringMethodDictionary_.insert(std::make_pair(name, method)).second) {
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

	static void AddFactoryMethod(suede_typeid type, FactoryMethod method) {
		if (!instance.typeIDMethodDictionary_.insert(std::make_pair(type, method)).second) {
			Debug::LogError("failed to add factroy method for %zu.", type);
		}
	}

	static Factory instance;
	NameMethodDictionary stringMethodDictionary_;
	TypeIDMethodDictionary typeIDMethodDictionary_;
	FactoryMethod methodArray_[ObjectType::size()];
};
