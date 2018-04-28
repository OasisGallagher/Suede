Properties { }

SubShader {
	Pass {
		Cull Off;
		Blend Off;
		ZWrite Off;
		ZTest Off;
	
		GLSLPROGRAM

		#stage vertex
		#include "include/post_effect_vertex.inc"

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;

		uniform sampler2D c_mainTexture;

		void main() {
			fragColor = texture(c_mainTexture, texCoord);
		}

		ENDGLSL
	}
}
