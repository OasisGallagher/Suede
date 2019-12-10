#pragma once
#include <map>
#include "shader.h"
#include "tools/string.h"
#include "tools/singleton.h"
#include "internal/base/uniformbuffer.h"

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

class UniformBuffer;
class UniformBufferManager : public Singleton<UniformBufferManager> {
	friend Singleton<UniformBufferManager>;

public:
	uint GetOffsetAlignment() { return offsetAlignment_; }

public:
	void Attach(Shader shader);
	bool Update(const std::string& name, const void* data, uint offset, uint size);

private:
	UniformBufferManager();
	~UniformBufferManager();

private:
	template <class T>
	void CreateBuffer(uint size = 0);

private:
	typedef std::map<std::string, UniformBuffer*> SharedUniformBufferContainer;

private:
	uint offsetAlignment_;
	SharedUniformBufferContainer sharedUniformBuffers_;
};

template  <class T>
void UniformBufferManager::CreateBuffer(uint size) {
	if (size == 0) { size = sizeof(T); }
	UniformBuffer* ptr = MEMORY_NEW(UniformBuffer);
	ptr->Create(T::GetName(), size);
	sharedUniformBuffers_.insert(std::make_pair(T::GetName(), ptr));
}
