#ifndef _LIT_VERTEX_INCLUDED
#define _LIT_VERTEX_INCLUDED

out vec4 _ShadowCoord;
#define _CalculateShadowCoord()	_ShadowCoord = _WorldToShadowMatrix * _LocalToWorldMatrix * vec4(_Pos, 1)

#endif // _LIT_VERTEX_INCLUDED
