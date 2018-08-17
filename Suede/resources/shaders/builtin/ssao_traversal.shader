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
		in vec3 _Normal;

		out vec3 cameraSpacePos;
		out vec3 cameraSpaceNormal;

		void main() {
			cameraSpacePos = (_WorldToCameraMatrix * _LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
			cameraSpaceNormal = (transpose(inverse(_WorldToCameraMatrix * _LocalToWorldMatrix)) * vec4(_Pos, 1)).xyz;
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		in vec3 cameraSpacePos;
		in vec3 cameraSpaceNormal;

		out vec3 pos;
		out vec3 normal;

		void main() {
			pos = cameraSpacePos;
			normal = cameraSpaceNormal;
		}

		ENDGLSL
	}
}
