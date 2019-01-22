#pragma once

#include <map>
#include <vector>
#include <functional>

#include "object.h"
#include "debug/debug.h"
#include "memory/memory.h"

class Factory {
	typedef std::function<Object()> FactoryMethod;
	typedef std::map<suede_guid, FactoryMethod> TypeIDMethodDictionary;
	typedef std::map<std::string, FactoryMethod> NameMethodDictionary;

	Factory();

public:
	static Object Create(const std::string& name) {
		NameMethodDictionary::iterator pos = instance.stringMethodDictionary_.find(name);
		if (pos == instance.stringMethodDictionary_.end()) {
			Debug::LogError("no factroy method exists for %s", name.c_str());
			return nullptr;
		}

		return pos->second();
	}

	static Object Create(suede_guid guid) {
		TypeIDMethodDictionary::iterator pos = instance.typeIDMethodDictionary_.find(guid);
		if (pos == instance.typeIDMethodDictionary_.end()) {
			Debug::LogError("no factroy method exists for type %zu.", guid);
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

	static bool AddFactoryMethod(const std::string& name, const FactoryMethod& method) {
		if (!instance.stringMethodDictionary_.insert(std::make_pair(name, method)).second) {
			Debug::LogError("failed to add factroy method for %s", name.c_str());
			return false;
		}

		return true;
	}

	static bool AddFactoryMethod(ObjectType type, const FactoryMethod& method) {
		if (instance.methodArray_[(int)type] != nullptr) {
			Debug::LogError("method for type %d already exists.", type);
			return false;
		}

		instance.methodArray_[(int)type] = method;
		return true;
	}

	static bool AddFactoryMethod(suede_guid guid, const FactoryMethod& method) {
		if (!instance.typeIDMethodDictionary_.insert(std::make_pair(guid, method)).second) {
			Debug::LogError("failed to add factroy method for %zu.", guid);
			return false;
		}

		return true;
	}

private:
	NameMethodDictionary stringMethodDictionary_;
	TypeIDMethodDictionary typeIDMethodDictionary_;
	FactoryMethod methodArray_[ObjectType::size()];

	static Factory instance;
};
