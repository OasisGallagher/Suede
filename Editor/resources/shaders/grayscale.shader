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
			vec3 scale = vec3(0.2126, 0.7152, 0.0722);
			float average = dot(scale, texture(_MainTexture, texCoord).rgb);
			fragColor = vec4(average, average, average, 1);
		}

		ENDGLSL
	}
}
