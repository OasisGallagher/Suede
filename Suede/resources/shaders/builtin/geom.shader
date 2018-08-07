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
		out vec3 worldPos;

		void main() {
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		in vec3 worldPos;
		out vec3 fragColor;

		void main() {
			fragColor = worldPos;
		}

		ENDGLSL
	}
}