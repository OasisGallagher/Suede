#shader vertex
in vec3 c_position;

uniform mat4 c_localToClipSpaceMatrix;

void main() {
	gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
}

#shader fragment
out float c_depth;

void main() {
	c_depth = gl_FragCoord.z;
}
