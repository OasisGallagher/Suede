#shader vertex
in vec3 c_position;
in vec2 c_texCoord;

out vec2 texCoord;

uniform mat4 c_localToClipSpaceMatrix;
uniform mat4 c_localToWorldSpaceMatrix;

void main() {
	texCoord = c_texCoord;
	gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
}

#shader fragment
out vec4 fragColor;

in vec2 texCoord;

uniform vec4 c_mainColor;
uniform sampler2D c_mainTexture;

void main() {
	fragColor = texture(c_mainTexture, texCoord) * c_mainColor;
}
