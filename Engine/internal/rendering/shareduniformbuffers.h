#pragma once
#include <map>
#include "tools/string.h"
#include "internal/base/uniformbuffer.h"
#include "internal/base/shaderinternal.h"

#define DEFINE_SHARED_UNIFORM_BUFFER(name, ...) \
	struct name { \
		__VA_ARGS__ \
		static const char* GetName() { return #name; } \
	}

DEFINE_SHARED_UNIFORM_BUFFER(SharedTimeUniformBuffer,
	Vector4 time;
);

DEFINE_SHARED_UNIFORM_BUFFER(SharedLightUniformBuffer,
	struct {
		Vector3 color;
		float density;
	} fogParams;

	Vector4 ambientColor;

	Vector4 lightPos;
	Vector4 lightDir;
	Vector4 lightColor;
);

DEFINE_SHARED_UNIFORM_BUFFER(SharedTransformsUniformBuffer,
	Matrix4 worldToClipMatrix;
	Matrix4 worldToCameraMatrix;
	Matrix4 cameraToClipMatrix;
	Matrix4 worldToShadowMatrix;
	Vector4 cameraPos;
	Vector4 projParams;
	Vector4 screenParams;
);

#undef DEFINE_SHARED_UNIFORM_BUFFER

class Context;
class UniformBuffer;
class SharedUniformBuffers {
public:
	SharedUniformBuffers(Context* context);
	~SharedUniformBuffers();

public:
	void Attach(ShaderInternal* shader);
	bool UpdateUniformBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	template <class T>
	void CreateBuffer(Context* context, uint size = 0);

	void Destroy();

private:
	typedef std::map<std::string, UniformBuffer*> Container;

private:
	Context* context_;
	Container uniformBuffers_;
};

template  <class T>
void SharedUniformBuffers::CreateBuffer(Context* context, uint size) {
	if (size == 0) { size = sizeof(T); }
	UniformBuffer* buffer = uniformBuffers_[T::GetName()] = new UniformBuffer(context);
	buffer->Create(T::GetName(), size);
}
