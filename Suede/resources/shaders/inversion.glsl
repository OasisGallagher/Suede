#shader vertex
#include "shaders/include/post_effect_vertex.inc"

#shader fragment
out vec4 c_fragColor;

in vec2 texCoord;

uniform sampler2D c_mainTexture;

void main() {
	c_fragColor = vec4(1 - texture(c_mainTexture, texCoord).rgb, 1);
}
