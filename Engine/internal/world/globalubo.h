#pragma once
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
	};
}

class GlobalUBO {
public:
	enum { MaxEntityMatrixBuffers = 10 };

public:
	static GlobalUBO* Get();

public:
	static void AttachSharedBuffer(Shader shader);
	static void AttachEntityBuffer(Shader shader, uint offset, uint size);
	static bool SetBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	GlobalUBO();
	~GlobalUBO();
};
