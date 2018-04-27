#include "tools/math2.h"
#include "memory/memory.h"
#include "uniformbuffermanager.h"

uint UniformBufferManager::maxBlockSize_;
uint UniformBufferManager::offsetAlignment_;

UniformBufferManager::SharedUniformBufferContainer UniformBufferManager::sharedUniformBuffers_;

void UniformBufferManager::Initialize() {
	GL::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (GLint*)&offsetAlignment_);
	GL::GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, (GLint*)&maxBlockSize_);

	CreateSharedUniformBuffer<SharedTimeUniformBuffer>();
	CreateSharedUniformBuffer<SharedLightUniformBuffer>();
	CreateSharedUniformBuffer<SharedTransformsUniformBuffer>();

	CreateSharedUniformBuffer<EntityMatricesUniforms>(GetMaxBlockSize());
}

void UniformBufferManager::Destroy() {
	for (SharedUniformBufferContainer::iterator ite = sharedUniformBuffers_.begin(); ite != sharedUniformBuffers_.end(); ++ite) {
		MEMORY_RELEASE(ite->second);
	}
}

void UniformBufferManager::AttachSharedBuffers(Shader shader) {
	for (SharedUniformBufferContainer::iterator ite = sharedUniformBuffers_.begin(); ite != sharedUniformBuffers_.end(); ++ite) {
		ite->second->AttachBuffer(shader);
	}
}

bool UniformBufferManager::UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size) {
	SharedUniformBufferContainer::iterator pos = sharedUniformBuffers_.find(name);
	if (pos == sharedUniformBuffers_.end()) {
		Debug::LogError("invalid shared uniform buffer name %s.", name.c_str());
		return false;
	}

	pos->second->UpdateBuffer(data, offset, size);
	return true;
}

const std::string GetEntityMatricesUniformBufferName(int i) {
	return String::Format("EntityMatrices[%d]", i);
}
