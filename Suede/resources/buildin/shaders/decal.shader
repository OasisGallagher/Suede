Properties { }

SubShader {
	Pass {
		Offset -10 0;
		Cull Back;
		ZWrite On;
		ZTest LEqual;
		//Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;
		
		out vec3 uvw;

		uniform mat4 c_decalMatrix;
		uniform mat4 c_localToClipSpaceMatrix;

		void main() {
			uvw = (c_decalMatrix * vec4(c_position, 1)).xyw;
			gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec3 uvw;
		uniform sampler2D c_mainTexture;

		void main() {
			/*
			vec2 uv = uvw.xy / uvw.z;
			uv = uv * 0.5 + 0.5;
			fragColor = texture(c_mainTexture, uv);
			*/
			fragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1);
		}

		ENDGLSL
	}
}
