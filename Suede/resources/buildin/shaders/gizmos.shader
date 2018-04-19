Properties { }

SubShader {
	Pass {
		GLSLPROGRAM

		#stage vertex
		#include "buildin/shaders/include/suede.inc"

		in vec3 c_position;

		void main() {
			gl_Position = c_worldToClipMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec3 fragColor;
		uniform vec4 c_mainColor;

		void main() {
			fragColor = c_mainColor.xyz;
		}

		ENDGLSL
	}
}
