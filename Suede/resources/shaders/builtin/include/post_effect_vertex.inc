#ifndef __POST_EFFECT_VERTEX_INC__
#define __POST_EFFECT_VERTEX_INC__

in vec3 _Pos;
in vec2 _TexCoord;

out vec2 texCoord;

void main() {
	texCoord = _TexCoord;
	gl_Position = vec4(_Pos, 1);
}

#endif	// __POST_EFFECT_VERTEX_INC__
