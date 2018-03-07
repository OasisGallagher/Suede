// uniform buffers.
layout(std140) uniform Time {
	float time;
	float deltaTime;
};

layout(std140) uniform Light {
	vec3 c_ambientLightColor;
	vec3 c_lightColor;
	vec3 c_lightPosition;
	vec3 c_lightDirection;
};

layout(std140) uniform Transforms {
	mat4 c_worldToClipSpaceMatrix;
	mat4 c_worldToCameraSpaceMatrix;
	mat4 c_cameraToClipSpaceMatrix;

	vec3 c_cameraPosition;
};

layout(std140) uniform EntityMatrices {
	mat4 localToWorldSpaceMatrix;
	mat4 localToClipSpaceMatrix;
} c_entityMatrices[10];

uniform int c_index;

#define SUEDE_MVP	c_entityMatrices[c_index / 512].localToClipSpaceMatrix
#define SUEDE_WORLD	c_entityMatrices[c_index / 512].localToWorldSpaceMatrix
