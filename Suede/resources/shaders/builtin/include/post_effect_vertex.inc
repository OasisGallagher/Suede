#ifndef _POST_EFFECT_VERTEX_INCLUDED
#define _POST_EFFECT_VERTEX_INCLUDED

#include "builtin/include/test_include_nested.inc"

void main() {
	texCoord = _TexCoord;
	gl_Position = vec4(_Pos, 1);
}

#endif	// _POST_EFFECT_VERTEX_INCLUDED
