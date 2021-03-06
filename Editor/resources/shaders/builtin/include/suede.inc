#ifndef _SUEDE_INCLUDED
#define _SUEDE_INCLUDED

// ========= Shared Uniform Buffers =========
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

// ========= Uniforms =========
#ifdef _VERTEX_SHADER

in int _MatrixTextureBufferOffset;
uniform samplerBuffer _MatrixTextureBuffer;

#define __F(offset)	texelFetch(_MatrixTextureBuffer, _MatrixTextureBufferOffset + gl_InstanceID * 8 + offset)

#define _LocalToWorldMatrix		mat4(__F(0), __F(1), __F(2), __F(3))
#define _LocalToClipMatrix		mat4(__F(4), __F(5), __F(6), __F(7))

#endif	// _VERTEX_SHADER

#ifdef _FRAGMENT_SHADER

#endif	// _FRAGMENT_SHADER

// ========= Functions =========

float saturate(float x) {
    return clamp(x, 0, 1);
}

// --------- Vertex Shader Functions ---------

#ifdef _VERTEX_SHADER

// initialize tangent to world space matrix.
#define _TANGENT_TO_WORLD_ROTATION(mat) \
			mat = mat3( \
					normalize(mat3(_LocalToWorldMatrix) * _Tangent), \
					vec3(0), \
					normalize(mat3(_LocalToWorldMatrix) * _Normal)); \
			mat[1] = normalize(cross(mat[2], mat[0]))

#define _DEF_TANGENT_TO_WORLD_ROTATION(variableName) \
		mat3 variableName; \
		_TANGENT_TO_WORLD_ROTATION(variableName)
		
#endif	// _VERTEX_SHADER

// --------- Fragment Shader Functions ---------
#ifdef _FRAGMENT_SHADER

float _LinearEyeDepth(float z) {
	z = 2 * z - 1;
	return (_ProjParams.x + _ProjParams.y) / (z * (_ProjParams.x - _ProjParams.y) - 2);
}

float _Linear01Depth(float z) {
	return _LinearEyeDepth(z) / _ProjParams.y;
}

vec2 _FragTexCoord() {
	return gl_FragCoord.xy / _ScreenParams.xy;
}

#endif	// _FRAGMENT_SHADER

#endif	// _SUEDE_INCLUDED

