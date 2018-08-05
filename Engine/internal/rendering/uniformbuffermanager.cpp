#include "tools/math2.h"
#include "memory/memory.h"
#include "../api/glutils.h"
#include "uniformbuffermanager.h"

UniformBufferManager::UniformBufferManager() {
	GL::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (GLint*)&offsetAlignment_);

	CreateSharedUniformBuffer<SharedTimeUniformBuffer>();
	CreateSharedUniformBuffer<SharedLightUniformBuffer>();
	CreateSharedUniformBuffer<SharedTransformsUniformBuffer>();
}

UniformBufferManager::~UniformBufferManager() {
	for (SharedUniformBufferContainer::iterator ite = sharedUniformBuffers_.begin(); ite != sharedUniformBuffers_.end(); ++ite) {
		MEMORY_DELETE(ite->second);
	}
}

void UniformBufferManager::AttachSharedBuffers(Shader shader) {
	for (SharedUniformBufferContainer::iterator ite = sharedUniformBuffers_.begin(); ite != sharedUniformBuffers_.end(); ++ite) {
		ite->second->AttachBuffer(shader);
	}
}

bool UniformBufferManager::UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size) {
	if (size > GLUtils::GetLimits(GLLimitsMaxUniformBlockSize)) {
		Debug::LogError("%d exceeds max buffer size.", size);
		return false;
	}

	SharedUniformBufferContainer::iterator pos = sharedUniformBuffers_.find(name);
	if (pos == sharedUniformBuffers_.end()) {
		Debug::LogError("invalid shared uniform buffer name %s.", name.c_str());
		return false;
	}

	pos->second->UpdateBuffer(data, offset, size);
	return true;
}
