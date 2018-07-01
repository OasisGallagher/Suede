#pragma once
#include <map>
#include "shader.h"
#include "tools/string.h"
#include "internal/base/uniformbuffer.h"

struct SharedTimeUniformBuffer {
	glm::vec4 time;

	static const char* GetName() { return "SharedTimeUniformBuffer"; }
};

struct SharedLightUniformBuffer {
	glm::vec4 ambientLightColor;
	glm::vec4 lightColor;
	glm::vec4 lightPosition;
	glm::vec4 lightDirection;

	static const char* GetName() { return "SharedLightUniformBuffer"; }
};

struct SharedTransformsUniformBuffer {
	glm::mat4 worldToClipMatrix;
	glm::mat4 worldToCameraMatrix;
	glm::mat4 cameraToClipMatrix;
	glm::mat4 worldToShadowMatrix;
	glm::vec4 cameraPosition;

	static const char* GetName() { return "SharedTransformsUniformBuffer"; }
};

class UniformBuffer;
class UniformBufferManager {
public:
	static void Initialize();
	static void Destroy();

public:
	static uint GetOffsetAlignment() { return offsetAlignment_; }

public:
	static void AttachSharedBuffers(Shader shader);
	static bool UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	UniformBufferManager() {}

private:
	template <class T>
	static void CreateSharedUniformBuffer(uint size = 0);

private:
	typedef std::map<std::string, UniformBuffer*> SharedUniformBufferContainer;
	static SharedUniformBufferContainer sharedUniformBuffers_;
	static uint offsetAlignment_;
};

template  <class T>
void UniformBufferManager::CreateSharedUniformBuffer(uint size) {
	if (size == 0) { size = sizeof(T); }
	UniformBuffer* ptr = MEMORY_CREATE(UniformBuffer);
	ptr->Create(T::GetName(), size);
	sharedUniformBuffers_.insert(std::make_pair(T::GetName(), ptr));
}
