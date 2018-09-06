Properties { }

SubShader {
	Pass {
		Cull Front;
		ZWrite On;
		ZTest LEqual;

		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;

		uniform mat4 _WorldToOrthographicLightMatrix;

		void main() {
			gl_Position = _WorldToOrthographicLightMatrix * _LocalToWorldMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out float depth;

		void main() {
			depth = gl_FragCoord.z;
		}

		ENDGLSL
	}
}