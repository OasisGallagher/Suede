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
	glm::vec4 time;
);

DEFINE_SHARED_UNIFORM_BUFFER(SharedLightUniformBuffer,
	struct {
		glm::vec3 color;
		float density;
	} fog;

	glm::vec4 ambientColor;

	glm::vec4 lightPos;
	glm::vec4 lightDir;
	glm::vec4 lightColor;
);

DEFINE_SHARED_UNIFORM_BUFFER(SharedTransformsUniformBuffer,
	glm::mat4 worldToClipMatrix;
	glm::mat4 worldToCameraMatrix;
	glm::mat4 cameraToClipMatrix;
	glm::mat4 worldToShadowMatrix;
	glm::vec4 cameraPos;
);

#undef DEFINE_SHARED_UNIFORM_BUFFER

class UniformBuffer;
class UniformBufferManager : public Singleton<UniformBufferManager> {
	friend Singleton<UniformBufferManager>;

public:
	uint GetOffsetAlignment() { return offsetAlignment_; }

public:
	void AttachSharedBuffers(Shader shader);
	bool UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	UniformBufferManager();
	~UniformBufferManager();

private:
	template <class T>
	void CreateSharedUniformBuffer(uint size = 0);

private:
	typedef std::map<std::string, UniformBuffer*> SharedUniformBufferContainer;

private:
	uint offsetAlignment_;
	SharedUniformBufferContainer sharedUniformBuffers_;
};

template  <class T>
void UniformBufferManager::CreateSharedUniformBuffer(uint size) {
	if (size == 0) { size = sizeof(T); }
	UniformBuffer* ptr = MEMORY_NEW(UniformBuffer);
	ptr->Create(T::GetName(), size);
	sharedUniformBuffers_.insert(std::make_pair(T::GetName(), ptr));
}
