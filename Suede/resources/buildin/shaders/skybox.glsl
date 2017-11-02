#shader vertex

layout(location = 0) in vec3 c_position;

uniform mat4 c_localToClipSpaceMatrix;

out vec3 texCoord;

void main() {
	vec4 pos = c_localToClipSpaceMatrix * vec4(c_position, 1);
	gl_Position = pos.xyww;
	texCoord = c_position;
}

#shader fragment

out vec4 color;

in vec3 texCoord;

uniform samplerCube c_mainTexture;

void main() {
	color = texture(c_mainTexture, texCoord);
}
