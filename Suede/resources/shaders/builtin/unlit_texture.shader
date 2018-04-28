Properties { }

SubShader {
	Pass {
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 c_position;
		in vec2 c_texCoord;

		out vec2 texCoord;

		void main() {
			texCoord = c_texCoord;
			gl_Position = c_localToClipMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		uniform vec4 c_mainColor;
		uniform sampler2D c_mainTexture;

		void main() {
			fragColor = texture(c_mainTexture, texCoord) * c_mainColor;
		}

		ENDGLSL
	}
}
