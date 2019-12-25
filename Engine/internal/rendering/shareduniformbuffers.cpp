#include "shareduniformbuffers.h"

#include "internal/base/context.h"

SharedUniformBuffers::SharedUniformBuffers(Context* context) : context_(context) {
	CreateBuffer<SharedTimeUniformBuffer>(context);
	CreateBuffer<SharedLightUniformBuffer>(context);
	CreateBuffer<SharedTransformsUniformBuffer>(context);
	context->destroyed.subscribe(this, &SharedUniformBuffers::OnContextDestroyed);
}

SharedUniformBuffers::~SharedUniformBuffers() {
	if (context_ != nullptr) {
		context_->destroyed.unsubscribe(this);
	}

	Destroy();
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

void SharedUniformBuffers::Destroy() {
	for (auto ite = uniformBuffers_.begin(); ite != uniformBuffers_.end(); ++ite) {
		delete ite->second;
	}

	uniformBuffers_.clear();
}

void SharedUniformBuffers::OnContextDestroyed() {
	Destroy();
	context_ = nullptr;
}
