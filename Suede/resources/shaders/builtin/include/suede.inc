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

float saturate(float x) {
    return clamp(x, 0, 1);
}

// --------- Vertex Shader Functions 

#ifdef _VERTEX_SHADER
#define _TANGENT_SPACE_ROTATION(mat) \
			mat = mat3((_LocalToWorldMatrix * vec4(_Normal, 0)).xyz, \
				(_LocalToWorldMatrix * vec4(_Tangent, 0)).xyz, \
				vec3(0)); \
			mat[2] = cross(mat[0], mat[1])

#define _DEF_TANGENT_SPACE_ROTATION(variableName) \
		mat3 variableName; \
		_TANGENT_SPACE_ROTATION(variableName)
		
#endif

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

