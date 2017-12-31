#stage vertex
#include "shaders/include/post_effect_vertex.inc"

#stage fragment
out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D c_mainTexture;

void main() {
	fragColor = vec4(1 - texture(c_mainTexture, texCoord).rgb, 1);
}
