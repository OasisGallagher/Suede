#pragma once
#include <map>
#include "tools/string.h"
#include "internal/base/uniformbuffer.h"
#include "internal/base/shaderinternal.h"

struct SharedTimeUniformBuffer {
	Vector4 time;
	static const char* GetName() { return "SharedTimeUniformBuffer"; }
};

struct SharedLightUniformBuffer {
	struct {
		Vector3 color;
		float density;
	} fogParams;

	Vector4 ambientColor;

	Vector4 lightPos;
	Vector4 lightDir;
	Vector4 lightColor;

	static const char* GetName() { return "SharedLightUniformBuffer"; }
};

struct SharedTransformsUniformBuffer {
	Matrix4 worldToClipMatrix;
	Matrix4 worldToCameraMatrix;
	Matrix4 cameraToClipMatrix;
	Matrix4 worldToShadowMatrix;
	Vector4 cameraPos;
	Vector4 projParams;
	Vector4 screenParams;
	static const char* GetName() { return "SharedTransformsUniformBuffer"; }
};

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
