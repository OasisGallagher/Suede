Properties { 
	float sampleRadius = 0.5;
}

SubShader {
	Pass {
		Cull Off;
		Blend Off;
		ZWrite Off;
		ZTest Off;

		GLSLPROGRAM
		#stage vertex

		out vec2 texCoord;

		void main() {
			texCoord = (_Pos.xy + 1) / 2;
			gl_Position = vec4(_Pos, 1);
		}

		#stage fragment
		out vec3 fragColor;

		void main() {
			fragColor = worldPos;
		}

		ENDGLSL
	}
}
