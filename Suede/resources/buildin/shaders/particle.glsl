#shader vertex
in vec3 c_position;
in vec4 c_user0;
in vec4 c_user1;

out vec2 texCoord;
out vec4 color;

uniform mat4 c_worldToClipSpaceMatrix;
uniform mat4 c_worldToCameraSpaceMatrix;

void main() {
	float size = c_user1.w;
	vec3 center = c_user1.xyz;

	vec3 cameraUp = vec3(c_worldToCameraSpaceMatrix[0][1], c_worldToCameraSpaceMatrix[1][1], c_worldToCameraSpaceMatrix[2][1]);
	vec3 cameraRight = vec3(c_worldToCameraSpaceMatrix[0][0], c_worldToCameraSpaceMatrix[1][0], c_worldToCameraSpaceMatrix[2][0]);

	vec3 position = center + cameraUp * c_position.y * size + cameraRight * c_position.x * size;
	gl_Position = c_worldToClipSpaceMatrix * vec4(position, 1);

	texCoord = c_position.xy + vec2(0.5f);
	color = c_user0;
}

#shader fragment
in vec2 texCoord;
in vec4 color;

out vec4 c_fragColor;

uniform sampler2D c_mainTexture;

void main() {
	c_fragColor = texture(c_mainTexture, texCoord) * color; 
}
