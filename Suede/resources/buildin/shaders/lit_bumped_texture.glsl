#shader vertex
in vec3 c_position;
in vec2 c_texCoord;
in vec3 c_normal;
in vec3 c_tangent;

out vec2 texCoord;
out vec3 worldPos;
out mat3 tangentToWorldSpaceMatrix;

uniform mat4 c_localToClipSpaceMatrix;
uniform mat4 c_localToWorldSpaceMatrix;

void main() {
	texCoord = c_texCoord;
	worldPos = (c_localToWorldSpaceMatrix * vec4(c_position, 1)).xyz;

	vec3 worldNormal = (c_localToWorldSpaceMatrix * vec4(c_normal, 0)).xyz;
	vec3 worldTangent = (c_localToWorldSpaceMatrix * vec4(c_tangent, 0)).xyz;
	vec3 worldBitangent = cross(worldNormal, worldTangent);
	
	vec3 bitangent = cross(c_normal, c_tangent);
	tangentToWorldSpaceMatrix = mat3(worldTangent, worldBitangent, worldNormal);

	gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
}

#shader fragment
out vec4 c_fragColor;

in vec2 texCoord;
in vec3 worldPos;
in mat3 tangentToWorldSpaceMatrix;

uniform sampler2D c_mainTexture;
uniform sampler2D c_bumpTexture;

#include "buildin/shaders/include/light.inc"

void main() {
	vec3 normal = texture(c_bumpTexture, texCoord).xyz;
	normal = tangentToWorldSpaceMatrix * normal;

	vec4 albedo = texture(c_mainTexture, texCoord);
	c_fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal)), 1);
}
