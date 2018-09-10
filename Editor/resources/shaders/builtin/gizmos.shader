Properties { }

SubShader {
	Pass {
		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		uniform mat4 localToWorldMatrix;

		void main() {
			gl_Position = _WorldToClipMatrix * localToWorldMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec3 fragColor;
		uniform vec4 _MainColor;

		void main() {
			fragColor = _MainColor.xyz;
		}

		ENDGLSL
	}
}
