#include "uniformbuffermanager.h"
#include "tools/math2.h"
#include "memory/memory.h"
#include "internal/base/uniformbuffer.h"

uint UniformBufferManager::maxBlockSize_;
uint UniformBufferManager::offsetAlignment_;

UniformBufferManager::EntityUBOContainer UniformBufferManager::entityUBOs_;
UniformBufferManager::SharedUBOContainer UniformBufferManager::sharedUBOs_;

void UniformBufferManager::Initialize() {
	GL::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (GLint*)&offsetAlignment_);
	GL::GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, (GLint*)&maxBlockSize_);

#define CREATE_UBO(name)	UniformBuffer* name ## Ptr = MEMORY_CREATE(UniformBuffer); \
	(name ## Ptr)->Create(SharedUBONames::name, sizeof(SharedUBOStructs::name)); \
	sharedUBOs_.insert(std::make_pair((name ## Ptr)->GetName(), (name ## Ptr)))

	CREATE_UBO(Time);
	CREATE_UBO(Light);
	CREATE_UBO(Transforms);

	UniformBuffer* ptr = MEMORY_CREATE(UniformBuffer);
	ptr->Create(SharedUBONames::EntityMatricesInstanced, GetMaxBlockSize());
	sharedUBOs_.insert(std::make_pair(ptr->GetName(), (ptr)));

 	for (int i = 0; i < MaxEntityMatrixBuffers; ++i) {
		UniformBuffer* ptr = MEMORY_CREATE(UniformBuffer);
 		ptr->Create(EntityUBONames::GetEntityMatricesName(i), GetMaxBlockSize());
 		entityUBOs_[i] = ptr;
 	}

#undef CREATE_UBO
}

void UniformBufferManager::Destroy() {
	for (SharedUBOContainer::iterator ite = sharedUBOs_.begin(); ite != sharedUBOs_.end(); ++ite) {
		MEMORY_RELEASE(ite->second);
	}

	for (int i = 0; i < MaxEntityMatrixBuffers; ++i) {
		MEMORY_RELEASE(entityUBOs_[i]);
	}
}

void UniformBufferManager::AttachSharedBuffers(Shader shader) {
	for (SharedUBOContainer::iterator ite = sharedUBOs_.begin(); ite != sharedUBOs_.end(); ++ite) {
		ite->second->AttachBuffer(shader);
	}
}

void UniformBufferManager::AttachEntityBuffer(Shader shader, uint index) {
	static uint stride = GetMaxBlockSize() / sizeof(EntityUBOStructs::EntityMatrices);
	static uint n = Math::Log2PowerOfTwo(stride);
	
	uint pos = index >> n;
	uint offset = (index & (stride - 1)) * sizeof(EntityUBOStructs::EntityMatrices);
	entityUBOs_[pos]->AttachSubBuffer(shader, offset, sizeof(EntityUBOStructs::EntityMatrices));
}

bool UniformBufferManager::UpdateSharedBuffer(const std::string& name, const void * data, uint offset, uint size) {
	SharedUBOContainer::iterator pos = sharedUBOs_.find(name);
	if (pos == sharedUBOs_.end()) {
		return false;
	}

	pos->second->SetBuffer(data, offset, size);
	return true;
}

bool UniformBufferManager::UpdateEntityBuffer(uint index, const void* data, uint offset, uint size) {
	if (index >= MaxEntityMatrixBuffers) {
		Debug::LogError("index out of range.");
		return false;
	}

	entityUBOs_[index]->SetBuffer(data, offset, size);
	return true;
}
