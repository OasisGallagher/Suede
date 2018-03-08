#include "ubomanager.h"
#include "tools/math2.h"
#include "memory/memory.h"
#include "internal/base/ubo.h"

uint UBOManager::maxBlockSize_;
uint UBOManager::offsetAlignment_;

UBOManager::EntityUBOContainer UBOManager::entityUBOs_;
UBOManager::SharedUBOContainer UBOManager::sharedUBOs_;

void UBOManager::Initialize() {
	GL::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (GLint*)&offsetAlignment_);
	GL::GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, (GLint*)&maxBlockSize_);

#define CREATE_UBO(name)	UBO* name ## Ptr = MEMORY_CREATE(UBO); \
	(name ## Ptr)->Create(SharedUBONames::name, sizeof(SharedUBOStructs::name)); \
	sharedUBOs_.insert(std::make_pair((name ## Ptr)->GetName(), (name ## Ptr)))

	CREATE_UBO(Time);
	CREATE_UBO(Light);
	CREATE_UBO(Transforms);

	for (int i = 0; i < MaxEntityMatrixBuffers; ++i) {
		UBO* ptr = MEMORY_CREATE(UBO);
		// TODO: maximum buffer size.
		ptr->Create(EntityUBONames::GetEntityMatricesName(i), 65536);
		entityUBOs_[i] = ptr;
	}

#undef CREATE_UBO
}

void UBOManager::AttachSharedBuffer(Shader shader) {
	for (SharedUBOContainer::iterator ite = sharedUBOs_.begin(); ite != sharedUBOs_.end(); ++ite) {
		ite->second->AttachSharedBuffer(shader);
	}

	for (int i = 0; i < MaxEntityMatrixBuffers; ++i) {
		entityUBOs_[i]->AttachSharedBuffer(shader);
	}
}

void UBOManager::SetEntityBuffer(uint index) {
	static uint stride = GetMaxBlockSize() / sizeof(EntityUBOStructs::EntityMatrices);
	static uint n = Math::Log2PowerOfTwo(stride);
	
	uint pos = index >> n;
	uint offset = (index & (stride - 1)) * sizeof(EntityUBOStructs::EntityMatrices);
	entityUBOs_[pos]->SetEntityBuffer(offset, sizeof(EntityUBOStructs::EntityMatrices));
}

bool UBOManager::UpdateSharedBuffer(const std::string& name, const void * data, uint offset, uint size) {
	SharedUBOContainer::iterator pos = sharedUBOs_.find(name);
	if (pos == sharedUBOs_.end()) {
		return false;
	}

	pos->second->SetBuffer(data, offset, size);
	return true;
}

bool UBOManager::UpdateEntityBuffer(uint index, const void* data, uint offset, uint size) {
	if (index >= MaxEntityMatrixBuffers) {
		Debug::LogError("index out of range.");
		return false;
	}

	entityUBOs_[index]->SetBuffer(data, offset, size);
	return true;
}
