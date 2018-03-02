#pragma once
#include "shader.h"

namespace GlobalUBONames {
	static const char* Time = "Time";
	static const char* Light = "Light";
	static const char* Transforms = "Transforms";
}

namespace GlobalUBOStructs {
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

class GlobalUBO {
public:
	static GlobalUBO* Get();

public:
	static void Attach(Shader shader);
	static bool SetBuffer(const std::string& name, const void* data, uint offset, uint size);

private:
	GlobalUBO();
	~GlobalUBO();
};
