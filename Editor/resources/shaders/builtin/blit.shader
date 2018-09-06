Properties { }

SubShader {
	Pass {
		Cull Off;
		Blend Off;
		ZWrite Off;
		ZTest Off;
	
		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/post_effect_vertex.inc"

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;

		uniform sampler2D _MainTexture;

		void main() {
			fragColor = texture(_MainTexture, texCoord);
		}

		ENDGLSL
	}
}
