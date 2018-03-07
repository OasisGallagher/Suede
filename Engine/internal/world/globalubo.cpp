#include <map>
#include "globalubo.h"
#include "memory/memory.h"
#include "internal/base/ubo.h"

typedef std::map<std::string, UBO*> UBOContainer;
static UBOContainer ubos_;

GlobalUBO* GlobalUBO::Get() {
	static GlobalUBO instance;
	return &instance;
}

GlobalUBO::GlobalUBO() {
#define CREATE_UBO(name)	UBO* name ## Ptr = MEMORY_CREATE(UBO); \
	(name ## Ptr)->Create(SharedUBONames::name, sizeof(SharedUBOStructs::name)); \
	ubos_.insert(std::make_pair((name ## Ptr)->GetName(), (name ## Ptr)))

	CREATE_UBO(Time);
	CREATE_UBO(Light);
	CREATE_UBO(Transforms);

	for (int i = 0; i < MaxEntityMatrixBuffers; ++i) {
		UBO* ptr = MEMORY_CREATE(UBO);
		// TODO: maximum buffer size.
		ptr->Create(EntityUBONames::GetEntityMatricesName(i), 65536);
		ubos_.insert(std::make_pair(ptr->GetName(), ptr));
	}

#undef CREATE_UBO
}

GlobalUBO::~GlobalUBO() {
	for (UBOContainer::iterator ite = ubos_.begin(); ite != ubos_.end(); ++ite) {
		MEMORY_RELEASE(ite->second);
	}
}

void GlobalUBO::AttachSharedBuffer(Shader shader) {
	for (UBOContainer::iterator ite = ubos_.begin(); ite != ubos_.end(); ++ite) {
		// TODO: name pattern.
		if (!String::StartsWith(ite->first, "EntityMatrices")) {
			ite->second->AttachSharedBuffer(shader);
		}
	}
}

void GlobalUBO::AttachEntityBuffer(Shader shader, uint offset, uint size) {
	for (UBOContainer::iterator ite = ubos_.begin(); ite != ubos_.end(); ++ite) {
		// TODO: name pattern.
		if (String::StartsWith(ite->first, "EntityMatrices")) {
			ite->second->AttachEntityBuffer(shader, offset, size);
		}
	}
}

bool GlobalUBO::SetBuffer(const std::string & name, const void * data, uint offset, uint size) {
	UBOContainer::iterator pos = ubos_.find(name);
	if (pos == ubos_.end()) {
		return false;
	}

	pos->second->SetBuffer(data, offset, size);
	return true;
}
