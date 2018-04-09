#pragma once
#include <map>
#include "shader.h"
#include "tools/string.h"

namespace SharedUBONames {
	static const char* Time = "Time";
	static const char* Light = "Light";
	static const char* Transforms = "Transforms";
	static const char* EntityMatricesInstanced = "EntityMatricesInstanced";
}

namespace EntityUBONames {
	static const std::string GetEntityMatricesName(int i) {
		return String::Format("EntityMatrices[%d]", i);
	}
}

namespace SharedUBOStructs {
	struct Light {
		glm::vec4 ambientLightColor;
		glm::vec4 lightColor;
		glm::vec4 lightPosition;
		glm::vec4 lightDirection;
	};

	struct Transforms {
		glm::mat4 worldToClipMatrix;
		glm::mat4 worldToCameraMatrix;
		glm::mat4 cameraToClipMatrix;
		glm::vec4 cameraPosition;
	};

	struct Time {
		glm::vec4 time;
	};
}

namespace EntityUBOStructs {
	struct EntityMatrices {
		glm::mat4 localToWorldMatrix;
		glm::mat4 localToClipMatrix;
	};
}

class UniformBuffer;
class UniformBufferManager {
public:
	enum { MaxEntityMatrixBuffers = 20 };

public:
	static void Initialize();
	static void Destroy();

public:
	static uint GetMaxBlockSize() { return maxBlockSize_; }
	static uint GetOffsetAlignment() { return offsetAlignment_; }

public:
	static void AttachSharedBuffers(Shader shader);
	static void AttachEntityBuffer(Shader shader, uint index);
	static bool UpdateEntityBuffer(uint index, const void* data, uint offset, uint size);
	static bool UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	UniformBufferManager() {}

private:
	typedef std::map<std::string, UniformBuffer*> SharedUBOContainer;
	static SharedUBOContainer sharedUBOs_;

	typedef UniformBuffer*(EntityUBOContainer)[UniformBufferManager::MaxEntityMatrixBuffers];
	static EntityUBOContainer entityUBOs_;

	static uint maxBlockSize_;
	static uint offsetAlignment_;
};
