Properties { }

SubShader {
	Tags { Queue = "Transparent"; }

	Pass {
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;
		in vec4 c_instanceColor;
		in vec4 c_instanceGeometry;
		
		out vec2 texCoord;
		out vec4 color;

		#include "buildin/include/suede.inc"

		void main() {
			float size = c_instanceGeometry.w;
			vec3 center = c_instanceGeometry.xyz;

			vec3 cameraUp = vec3(c_worldToCameraMatrix[0][1], c_worldToCameraMatrix[1][1], c_worldToCameraMatrix[2][1]);
			vec3 cameraRight = vec3(c_worldToCameraMatrix[0][0], c_worldToCameraMatrix[1][0], c_worldToCameraMatrix[2][0]);

			vec3 position = center + cameraUp * c_position.y * size + cameraRight * c_position.x * size;
			gl_Position = c_worldToClipMatrix * vec4(position, 1);

			texCoord = c_position.xy + vec2(0.5);
			color = c_instanceColor;
		}

		#stage fragment

		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec4 color;

		uniform sampler2D c_mainTexture;

		void main() {
			fragColor = texture(c_mainTexture, texCoord) * color; 
		}

		ENDGLSL
	}
}
