in vec3 c_position;
in vec2 c_texCoord;

out vec2 texCoord;

void main() {
	texCoord = c_texCoord;
	gl_Position = vec4(c_position, 1);
}
