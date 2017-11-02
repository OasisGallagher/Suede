#shader vertex
in vec3 c_position;
in vec2 c_texCoord;
in vec3 c_normal;

out vec2 texCoord;
out vec3 worldPos;
out vec3 normal;

#include "buildin/shaders/include/light_vertex.inc"

uniform mat4 c_localToClipSpaceMatrix;
uniform mat4 c_localToWorldSpaceMatrix;

void main() {
	texCoord = c_texCoord;

	normal = (c_localToWorldSpaceMatrix * vec4(c_normal, 0)).xyz;
	worldPos = (c_localToWorldSpaceMatrix * vec4(c_position, 1)).xyz;
	
	calculateShadowCoord();

	gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
}

#shader fragment
out vec4 c_fragColor;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normal;

uniform sampler2D c_mainTexture;

#include "buildin/shaders/include/light_fragment.inc"

void main() {
	vec4 albedo = texture(c_mainTexture, texCoord);
	float visibility = calculateShadowVisibility();
	c_fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal), visibility), 1);
}
