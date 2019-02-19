Properties {
	color _MainColor = { 1, 1, 1, 1 };
}

SubShader {
	Tags {
		Queue = "Overlay + 1";
	}

	Pass {
		GLSLPROGRAM

		#stage vertex

		#include "builtin/include/suede.inc"

		in vec3 _Pos;

		void main() {
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment

		out vec4 fragColor;
		uniform vec4 _MainColor;

		void main() {
			fragColor = _MainColor;
		}

		ENDGLSL
	}
}
