Properties {
	color _MainColor = { 1, 1, 1, 1 };
}

SubShader {
	Pass {
		Cull Front;
		ZTest LEqual;

		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		out vec3 texCoord;

		void main() {
			vec4 pos = _CameraToClipMatrix * vec4(mat3(_WorldToCameraMatrix) * _Pos, 1);
			gl_Position = pos.xyww;
			texCoord = _Pos;
		}

		#stage fragment

		in vec3 texCoord;
		out vec4 fragColor;

		uniform vec4 _MainColor;
		uniform samplerCube _MainTexture;

		void main() {
			fragColor = texture(_MainTexture, texCoord) * _MainColor;
		}

		ENDGLSL
	}
}
