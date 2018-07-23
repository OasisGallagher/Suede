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
		in vec2 texCoord;
		out vec4 fragColor;

		uniform bool horizontal;
		uniform sampler2D _MainTexture;

		const float weights[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
		
		void main() {
			vec2 offset = 1.f / textureSize(_MainTexture, 0);
			vec3 result = texture(_MainTexture, texCoord).rgb * weights[0];
	
			if(horizontal) {
				for(int i = 1; i < weights.length(); ++i) {
					result += texture(_MainTexture, texCoord + vec2(offset.x * i, 0)).rgb * weights[i];
					result += texture(_MainTexture, texCoord - vec2(offset.x * i, 0)).rgb * weights[i];
				}
			}
			else {
				for(int i = 1; i < weights.length(); ++i) {
					result += texture(_MainTexture, texCoord + vec2(0, offset.y * i)).rgb * weights[i];
					result += texture(_MainTexture, texCoord - vec2(0, offset.y * i)).rgb * weights[i];
				}
			}
			
			fragColor = vec4(result, 1);
		}

		ENDGLSL
	}
}
