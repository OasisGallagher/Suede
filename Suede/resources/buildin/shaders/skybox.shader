Properties {
}

SubShader {
	Pass {
		Cull Front;
		ZTest LEqual;

		GLSLPROGRAM

		#stage vertex
		#include "buildin/shaders/include/suede.inc"

		in vec3 c_position;
		out vec3 texCoord;

		void main() {
			vec4 pos = c_cameraToClipMatrix * vec4(mat3(c_worldToCameraMatrix) * c_position, 1);
			gl_Position = pos.xyww;
			texCoord = c_position;
		}

		#stage fragment

		out vec4 fragColor;
		in vec3 texCoord;

		uniform samplerCube c_mainTexture;
		uniform vec4 c_mainColor;

		void main() {
			fragColor = texture(c_mainTexture, texCoord) * c_mainColor;
		}

		ENDGLSL
	}
}