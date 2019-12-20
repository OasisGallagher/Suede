#include "shareduniformbuffers.h"

SharedUniformBuffers::SharedUniformBuffers(Context* context) {
	CreateBuffer<SharedTimeUniformBuffer>(context);
	CreateBuffer<SharedLightUniformBuffer>(context);
	CreateBuffer<SharedTransformsUniformBuffer>(context);
}

SharedUniformBuffers::~SharedUniformBuffers() {
	for (auto ite = uniformBuffers_.begin(); ite != uniformBuffers_.end(); ++ite) {
		delete ite->second;
	}
}

void SharedUniformBuffers::Attach(Shader* shader) {
	for (Container::iterator ite = uniformBuffers_.begin(); ite != uniformBuffers_.end(); ++ite) {
		ite->second->AttachBuffer(shader);
	}
}

bool SharedUniformBuffers::UpdateUniformBuffer(const std::string& name, const void* data, uint offset, uint size) {
	auto pos = uniformBuffers_.find(name);
	if (pos == uniformBuffers_.end()) {
		Debug::LogError("invalid shared uniform buffer name %s.", name.c_str());
		return false;
	}

	return pos->second->UpdateBuffer(data, offset, size);
}
