Properties { }

SubShader {
	Pass {
		Offset -10 0;
		ZTest LEqual;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;
		out vec3 uvw;

		uniform mat4 c_decalMatrix;
		uniform mat4 c_localToClipSpaceMatrix;

		void main() {
			gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
			vec4 p = c_decalMatrix * vec4(c_position, 1);
			uvw = p.xyw;
		}

		#stage fragment
		out vec4 fragColor;

		in vec3 uvw;
		uniform sampler2D c_mainTexture;

		void main() {
			vec2 uv = uvw.xy / uvw.z;
			uv = uv * 0.5 + 0.5;
			fragColor = texture(c_mainTexture, uv);
		}

		ENDGLSL
	}
}
