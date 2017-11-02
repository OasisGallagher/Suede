#shader vertex
#include "shaders/include/post_effect_vertex.inc"

#shader fragment
out vec4 c_fragColor;

in vec2 texCoord;

uniform sampler2D c_mainTexture;

void main() {
	vec3 scale = vec3(0.2126, 0.7152, 0.0722);
	float average = dot(scale, texture(c_mainTexture, texCoord).rgb);
	c_fragColor = vec4(average, average, average, 1);
}
