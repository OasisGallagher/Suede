#ifndef _POST_EFFECT_VERTEX_INCLUDED
#define _POST_EFFECT_VERTEX_INCLUDED

in vec3 _Pos;
in vec2 _TexCoord;

out vec2 texCoord;

void main() {
	texCoord = _TexCoord;
	gl_Position = vec4(_Pos, 1);
}

#endif	// _POST_EFFECT_VERTEX_INCLUDED
