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
		
		out vec4 projTexCoord;
		out vec4 clipPosition;

		uniform mat4 c_decalMatrix;
		uniform mat4 c_worldToClipMatrix;

		void main() {
			projTexCoord = c_decalMatrix * vec4(c_position, 1);
			gl_Position = c_worldToClipMatrix * vec4(c_position, 1);
			clipPosition = gl_Position;
		}

		#stage fragment
		out vec4 fragColor;

		in vec4 clipPosition;
		in vec4 projTexCoord;
		uniform sampler2D c_mainTexture;

		void main() {
			fragColor = textureProj(c_mainTexture, projTexCoord);
		}

		ENDGLSL
	}
}
