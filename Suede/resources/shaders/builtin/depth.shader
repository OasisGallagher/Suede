Properties { }

SubShader {
	Pass {
		Cull Back;
		ZWrite On;
		ZTest LEqual;

		GLSLPROGRAM
		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 c_position;

		void main() {
			gl_Position = c_localToClipMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out float depth;

		void main() {
		//	depth = gl_FragCoord.z;
		}

		ENDGLSL
	}
}