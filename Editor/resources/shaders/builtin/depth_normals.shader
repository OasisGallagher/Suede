Properties{}

SubShader {
	Pass {
		Cull Back;
		ZWrite On;
		ZTest LEqual;

		GLSLPROGRAM
		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		in vec3 _Normal;

		// Normal in camera space.
		out vec3 normal;

		void main() {
			normal = transpose(inverse(mat3(_WorldToCameraMatrix) * mat3(_LocalToWorldMatrix))) * _Normal;
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		in vec3 normal;
		out vec4 fragColor;

		void main() {
			fragColor = vec4(gl_FragCoord.z, normal);
		}

		ENDGLSL
	}
}
