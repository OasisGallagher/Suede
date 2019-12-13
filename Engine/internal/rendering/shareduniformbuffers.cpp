#include "shareduniformbuffers.h"

SharedUniformBuffers::SharedUniformBuffers() {
	CreateBuffer<SharedTimeUniformBuffer>();
	CreateBuffer<SharedLightUniformBuffer>();
	CreateBuffer<SharedTransformsUniformBuffer>();
}

void SharedUniformBuffers::Attach(Shader* shader) {
	for (Container::iterator ite = uniformBuffers_.begin(); ite != uniformBuffers_.end(); ++ite) {
		ite->second->AttachBuffer(shader);
	}
}

bool SharedUniformBuffers::UpdateUniformBuffer(const std::string& name, const void* data, uint offset, uint size) {
	UniformBuffer* ub = nullptr;
	if (!uniformBuffers_.get(name, ub)) {
		Debug::LogError("invalid shared uniform buffer name %s.", name.c_str());
		return false;
	}

	return ub->UpdateBuffer(data, offset, size);
}
