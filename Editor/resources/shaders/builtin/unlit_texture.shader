Properties { }

SubShader {
	Pass {
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		in vec2 _TexCoord0;

		out vec2 texCoord;

		void main() {
			texCoord = _TexCoord0;
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		uniform vec4 _MainColor;
		uniform sampler2D _MainTexture;

		void main() {
			fragColor = texture(_MainTexture, texCoord) * _MainColor;
		}

		ENDGLSL
	}
}
