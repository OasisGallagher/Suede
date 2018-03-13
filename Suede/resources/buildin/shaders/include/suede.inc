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
	mat4 c_worldToClipMatrix;
	mat4 c_worldToCameraMatrix;
	mat4 c_cameraToClipMatrix;

	vec3 c_cameraPosition;
};

//uniform mat4 c_localToWorldMatrix;
//uniform mat4 c_localToClipMatrix;

layout(std140) uniform EntityMatricesInstanced {
	mat4 entityMatrices[C_MAX_ENTITY_MATRCES];
};

#define c_localToWorldMatrix	entityMatrices[gl_InstanceID * 2]
#define c_localToClipMatrix	entityMatrices[gl_InstanceID * 2 + 1]
