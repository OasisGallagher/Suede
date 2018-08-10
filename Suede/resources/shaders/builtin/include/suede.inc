#ifndef _SUEDE_INCLUDED
#define _SUEDE_INCLUDED

// --------- Shared Uniform Buffers ---------
layout(std140) uniform SharedTimeUniformBuffer {
	float _Time;
	float _DeltaTime;
};

layout(std140) uniform SharedLightUniformBuffer {
	vec4 _FogParams;
	vec3 _AmbientColor;

	vec3 _LightPos;
	vec3 _LightDir;
	vec3 _LightColor;
};

layout(std140) uniform SharedTransformsUniformBuffer {
	mat4 _WorldToClipMatrix;
	mat4 _WorldToCameraMatrix;
	mat4 _CameraToClipMatrix;
	mat4 _WorldToShadowMatrix;

	vec3 _CameraPos;
	vec4 _ProjParams;
	vec4 _ScreenParams;
};

// --------- Uniforms ---------
#ifdef _VERTEX_SHADER

// 8 matches VertexAttribMatrixOffset.
layout(location = 8) in int _MatrixTextureBufferOffset;
uniform samplerBuffer _MatrixTextureBuffer;

#define __F(offset)	texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + offset)

#define _LocalToWorldMatrix		mat4(__F(0), __F(1), __F(2), __F(3))
#define _LocalToClipMatrix		mat4(__F(4), __F(5), __F(6), __F(7))

#endif	// _VERTEX_SHADER

#ifdef _FRAGMENT_SHADER

#endif	// _FRAGMENT_SHADER

// --------- Functions ---------
float _Linear01Depth(float z) {
	return _ProjParams.x / (z * _ProjParams.x - z * _ProjParams.y + _ProjParams.y);
	//return 2.0 * _ProjParams.x * _ProjParams.y / ((_ProjParams.y + _ProjParams.x - (2 * z - 1) * (_ProjParams.y - _ProjParams.x)) * _ProjParams.y);
}

float _LinearEyeDepth(float z) {
	return _CameraToClipMatrix[3][2] / (2 * z - 1 - _CameraToClipMatrix[2][2]);
	//return _ProjParams.x * _ProjParams.y / (z * _ProjParams.x - z * _ProjParams.y + _ProjParams.y);
	//return 2.0 * _ProjParams.x * _ProjParams.y / (_ProjParams.y + _ProjParams.x - (2 * z - 1) * (_ProjParams.y - _ProjParams.x));
}

#ifdef _FRAGMENT_SHADER

vec2 _FragTexCoord() {
	return gl_FragCoord.xy / _ScreenParams.xy;
}

#endif	// _FRAGMENT_SHADER

#endif	// _SUEDE_INCLUDED

