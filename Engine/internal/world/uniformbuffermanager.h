#pragma once
#include <map>
#include "shader.h"
#include "tools/string.h"
#include "internal/base/uniformbuffer.h"

//static const char* SharedTimeUniformBufferName = "Time";
//static const char* SharedLightUniformBufferName = "Light";
//static const char* SharedTransformsUniformBufferName = "Transforms";
//static const char* SharedEntityMatricesInstancedUniformBufferName = "EntityMatricesInstanced";

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
	glm::vec4 cameraPosition;

	static const char* GetName() { return "SharedTransformsUniformBuffer"; }
};

struct EntityMatricesUniforms {
	glm::mat4 localToWorldMatrix;
	glm::mat4 localToClipMatrix;

	/**
	 * @returns name of the container contains these uniforms.
	 */
	static const char* GetName() { return "SharedEntityMatricesUniformBuffer"; }
};

class UniformBuffer;
class UniformBufferManager {
public:
	static void Initialize();
	static void Destroy();

public:
	static uint GetMaxBlockSize() { return maxBlockSize_; }
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

	static uint maxBlockSize_;
	static uint offsetAlignment_;
};

template  <class T>
void UniformBufferManager::CreateSharedUniformBuffer(uint size) {
	if (size == 0) { size = sizeof(T); }
	UniformBuffer* ptr = MEMORY_CREATE(UniformBuffer);
	ptr->Create(T::GetName(), size);
	sharedUniformBuffers_.insert(std::make_pair(T::GetName(), ptr));
}
