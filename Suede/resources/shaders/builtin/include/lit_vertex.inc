#ifndef __LIT_VERTEX_INC__
#define __LIT_VERTEX_INC__

out vec4 _ShadowCoord;
#define calculateShadowCoord()	_ShadowCoord = _WorldToShadowMatrix * _LocalToWorldMatrix * vec4(_Pos, 1)

#endif // __LIT_VERTEX_INC__
