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

//uniform mat4 c_localToWorldSpaceMatrix;
//uniform mat4 c_localToClipSpaceMatrix;

layout(std140) uniform EntityMatricesInstanced {
	mat4 entityMatrices[C_MAX_ENTITY_MATRCES];
};

#define c_localToWorldSpaceMatrix	entityMatrices[gl_InstanceID * 2]
#define c_localToClipSpaceMatrix	entityMatrices[gl_InstanceID * 2 + 1]

