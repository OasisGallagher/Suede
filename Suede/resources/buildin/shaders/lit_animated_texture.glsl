#shader vertex

in vec3 c_position;
in vec2 c_texCoord;
in vec3 c_normal;

in ivec4 c_boneIndexes;
in vec4 c_boneWeights;

out vec2 texCoord;
out vec3 worldPos;
out vec3 normal;

#include "buildin/shaders/include/light_vertex.inc"

uniform mat4 c_localToClipSpaceMatrix;
uniform mat4 c_localToWorldSpaceMatrix;
uniform mat4 c_boneToRootSpaceMatrices[C_MAX_BONE_COUNT];

void main() {
	mat4 matrix = c_boneToRootSpaceMatrices[c_boneIndexes[0]] * c_boneWeights[0];
	matrix += c_boneToRootSpaceMatrices[c_boneIndexes[1]] * c_boneWeights[1];
	matrix += c_boneToRootSpaceMatrices[c_boneIndexes[2]] * c_boneWeights[2];
	matrix += c_boneToRootSpaceMatrices[c_boneIndexes[3]] * c_boneWeights[3];

	texCoord = c_texCoord;

	normal = (c_localToWorldSpaceMatrix * matrix * vec4(c_normal, 0)).xyz;
	worldPos = (c_localToWorldSpaceMatrix * matrix * vec4(c_position, 1)).xyz;

	//calculateShadowCoord();

	gl_Position = c_localToClipSpaceMatrix * matrix * vec4(c_position, 1);
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
	float visibility = 1; //calculateShadowVisibility();
	c_fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal), visibility), 1);
}
