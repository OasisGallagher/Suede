#pragma once
#include <map>
#include "shader.h"
#include "tools/string.h"

namespace SharedUBONames {
	static const char* Time = "Time";
	static const char* Light = "Light";
	static const char* Transforms = "Transforms";
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
		glm::mat4 worldToClipSpaceMatrix;
		glm::mat4 worldToCameraSpaceMatrix;
		glm::mat4 cameraToClipSpaceMatrix;
		glm::vec4 cameraPosition;
	};

	struct Time {
		glm::vec4 time;
	};
}

namespace EntityUBOStructs {
	struct EntityMatrices {
		glm::mat4 localToWorldSpaceMatrix;
		glm::mat4 localToClipSpaceMatrix;
		glm::mat4 __padding0;
		glm::mat4 __padding1;
	};
}

class UBO;
class UBOManager {
public:
	enum { MaxEntityMatrixBuffers = 20 };

public:
	static void Initialize();

public:
	static uint GetMaxBlockSize() { return maxBlockSize_; }
	static uint GetOffsetAlignment() { return offsetAlignment_; }

public:
	static void AttachSharedBuffer(Shader shader);
	static void SetEntityBuffer(uint index);
	static bool UpdateEntityBuffer(uint index, const void* data, uint offset, uint size);
	static bool UpdateSharedBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	UBOManager() {}

private:
	typedef std::map<std::string, UBO*> SharedUBOContainer;
	static SharedUBOContainer sharedUBOs_;

	typedef UBO*(EntityUBOContainer)[UBOManager::MaxEntityMatrixBuffers];
	static EntityUBOContainer entityUBOs_;

	static uint maxBlockSize_;
	static uint offsetAlignment_;
};
