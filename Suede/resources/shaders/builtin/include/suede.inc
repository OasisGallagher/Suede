#ifndef __SUEDE_INC__
#define __SUEDE_INC__

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
	mat4 c_worldToShadowMatrix;

	vec3 c_cameraPosition;
};

#ifdef VERTEX_SHADER

// 8 matches VertexAttribMatrixOffset.
layout(location = 8) in int c_matrixBufferOffset;
uniform samplerBuffer c_matrixBuffer;

#define c_localToWorldMatrix	mat4( \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 0), \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 1), \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 2), \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 3) \
								)

#define c_localToClipMatrix		mat4( \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 4), \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 5), \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 6), \
									texelFetch(c_matrixBuffer, c_matrixBufferOffset + gl_InstanceID * 8 + 7) \
								)
#endif	// VERTEX_SHADER

#endif	// __SUEDE_INC__
