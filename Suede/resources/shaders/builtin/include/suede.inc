#ifndef _SUEDE_INCLUDED
#define _SUEDE_INCLUDED

// shared uniform buffers.
layout(std140) uniform SharedTimeUniformBuffer {
	float _Time;
	float _DeltaTime;
};

layout(std140) uniform SharedLightUniformBuffer {
	vec3 _AmbientLightColor;
	vec3 _LightColor;
	vec3 _LightPosition;
	vec3 _LightDirection;
};

layout(std140) uniform SharedTransformsUniformBuffer {
	mat4 _WorldToClipMatrix;
	mat4 _WorldToCameraMatrix;
	mat4 _CameraToClipMatrix;
	mat4 _WorldToShadowMatrix;

	vec3 _CameraPosition;
};

#ifdef _VERTEX_SHADER

// 8 matches VertexAttribMatrixOffset.
layout(location = 8) in int _MatrixTextureBufferOffset;
uniform samplerBuffer _MatrixTextureBuffer;

#define _LocalToWorldMatrix	mat4( \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 0), \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 1), \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 2), \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 3) \
								)

#define _LocalToClipMatrix		mat4( \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 4), \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 5), \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 6), \
									texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + 7) \
								)
#endif	// _VERTEX_SHADER

#endif	// _SUEDE_INCLUDED
