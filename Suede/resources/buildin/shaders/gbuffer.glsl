#stage vertex

in vec3 c_position;
in vec2 c_texCoord;
in vec3 c_normal;

uniform mat4 c_localToClipSpaceMatrix;
uniform mat4 c_localToWorldSpaceMatrix;

out VertOut {
	out	vec3 worldPos;
	out	vec2 texCoord;
	out	vec3 worldNormal;
} vo;

void main() {
	gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
	vo.worldPos = (c_localToWorldSpaceMatrix * vec4(c_position, 1)).xyz;
	vo.texCoord = c_texCoord;

	mat3 m = inverse(transpose(mat3(c_localToWorldSpaceMatrix)));
	vo.worldNormal = m * c_normal;
}

#stage fragment

in VertOut {
	in	vec3 worldPos;
	in	vec2 texCoord;
	in	vec3 worldNormal;
} fi;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec3 albedo;
layout(location = 2) out vec3 worldNormal;

uniform sampler2D c_mainTexture;

void main() {
	worldPos = fi.worldPos;
	albedo = texture(c_mainTexture, fi.texCoord).rgb;
	worldNormal = normalize(fi.worldNormal);
}

