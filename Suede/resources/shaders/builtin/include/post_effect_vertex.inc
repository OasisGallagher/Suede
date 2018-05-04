#ifndef __POST_EFFECT_VERTEX_INC__
#define __POST_EFFECT_VERTEX_INC__

in vec3 c_position;
in vec2 c_texCoord;

out vec2 texCoord;

void main() {
	texCoord = c_texCoord;
	gl_Position = vec4(c_position, 1);
}

#endif	// __POST_EFFECT_VERTEX_INC__
