Properties { }

SubShader {
	Pass {
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		#include "buildin/shaders/include/suede.inc"

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

		uniform bool test;
		void main() {
			fragColor = texture(c_mainTexture, texCoord) * c_mainColor;
			if (!test) {
			//	fragColor = vec4(texCoord, 0, 1);
			}
		}

		ENDGLSL
	}
}
