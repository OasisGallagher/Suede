Properties { }

SubShader {
	Pass {
		Cull Back;
		ZWrite On;
		ZTest LEqual;

		GLSLPROGRAM
		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;

		void main() {
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out float depth;

		void main() {
			depth = gl_FragCoord.z;
		}

		ENDGLSL
	}
}