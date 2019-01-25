Properties {
}

SubShader {
	Pass {
		GLSLPROGRAM

		#stage vertex

		#include "builtin/include/suede.inc"

		in vec3 _Pos;

		void main() {
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
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
