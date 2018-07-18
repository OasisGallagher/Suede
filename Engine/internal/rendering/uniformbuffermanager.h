#pragma once
#include <map>
#include "shader.h"
#include "tools/string.h"
#include "tools/singleton.h"
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
class UniformBufferManager : public Singleton<UniformBufferManager> {
public:
	//void Destroy();

public:
	uint GetOffsetAlignment() { return offsetAlignment_; }

public:
	void AttachSharedBuffers(Shader shader);
	bool UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size);

public:
	UniformBufferManager();
	~UniformBufferManager();

private:
	template <class T>
	void CreateSharedUniformBuffer(uint size = 0);

private:
	typedef std::map<std::string, UniformBuffer*> SharedUniformBufferContainer;
	SharedUniformBufferContainer sharedUniformBuffers_;
	uint offsetAlignment_;
};

template  <class T>
void UniformBufferManager::CreateSharedUniformBuffer(uint size) {
	if (size == 0) { size = sizeof(T); }
	UniformBuffer* ptr = MEMORY_CREATE(UniformBuffer);
	ptr->Create(T::GetName(), size);
	sharedUniformBuffers_.insert(std::make_pair(T::GetName(), ptr));
}
