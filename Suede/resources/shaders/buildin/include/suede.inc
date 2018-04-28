// shared uniform buffers.
layout(std140) uniform SharedTimeUniformBuffer {
	float c_time;
	float c_deltaTime;
};

layout(std140) uniform SharedLightUniformBuffer {
	vec3 c_ambientLightColor;
	vec3 c_lightColor;
	vec3 c_lightPosition;
	vec3 c_lightDirection;
};

layout(std140) uniform SharedTransformsUniformBuffer {
	mat4 c_worldToClipMatrix;
	mat4 c_worldToCameraMatrix;
	mat4 c_cameraToClipMatrix;

	vec3 c_cameraPosition;
};

layout(std140) uniform SharedEntityMatricesUniformBuffer {
	// C_MAX_ENTITY_MATRICES = GL_MAX_UNIFORM_BLOCK_SIZE / sizeof(mat4).
	mat4 c_entityMatrices[C_MAX_ENTITY_MATRICES];
};

#define c_localToWorldMatrix	(c_entityMatrices[gl_InstanceID * 2])
#define c_localToClipMatrix		(c_entityMatrices[gl_InstanceID * 2 + 1])

