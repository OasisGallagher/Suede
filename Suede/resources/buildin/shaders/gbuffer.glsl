#shader vertex

in vec3 c_position;
in vec2 c_texCoord;
in vec3 c_normal;

uniform mat4 c_localToClipSpaceMatrix;
uniform mat4 c_localToWorldSpaceMatrix;

out PTN {
	vec3 worldPos;
	vec2 texCoord;
	vec3 worldNormal;
} outPtn;

void main() {
	gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
	outPtn.worldPos = (c_localToWorldSpaceMatrix * vec4(c_position, 1)).xyz;
	outPtn.texCoord = c_texCoord;
	outPtn.worldNormal = (c_localToWorldSpaceMatrix * vec4(c_normal, 0)).xyz;
}

#shader fragment

in PTN {
	vec3 worldPos;
	vec2 texCoord;
	vec3 worldNormal;
} inPtn;

layout(location = 0) out vec4 worldPos;
layout(location = 1) out vec4 color;
layout(location = 2) out vec4 worldNormal;

uniform sampler2D c_mainTexture;

void main() {
	/*
	worldPos = inPtn.worldPos;
	color = texture(c_mainTexture, inPtn.texCoord).rgb;
	worldNormal = normalize(inPtn.worldNormal);
	*/
	color = vec4(1, 0, 0, 1);
	worldNormal = vec4(1, 0, 0, 1);
	worldPos = vec4(1, 1, 0, 1);
}
