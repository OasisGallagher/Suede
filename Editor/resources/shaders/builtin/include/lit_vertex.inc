#ifndef _VERTEX_SHADER
	#error lit_vertex.inc must be included in vertex shader.
#endif

#ifndef _LIT_VERTEX_INCLUDED
#define _LIT_VERTEX_INCLUDED

// position in view space.
out vec3 _ViewPos;
#define _CALCULATE_FOG_PARAMS()		_ViewPos = (_WorldToCameraMatrix * _LocalToWorldMatrix * vec4(_Pos, 1)).xyz

out vec4 _ShadowCoord;
#define _CALCULATE_SHADOW_COORD()	_ShadowCoord = _WorldToShadowMatrix * _LocalToWorldMatrix * vec4(_Pos, 1)

#endif // _LIT_VERTEX_INCLUDED
