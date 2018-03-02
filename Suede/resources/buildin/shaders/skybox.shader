Properties {
}

SubShader {
	Pass {
		Blend Off;
		Cull Front;
		ZTest LEqual;
		DepthWrite Off;

		GLSLPROGRAM

		#stage vertex

		in vec3 c_position;
		out vec3 texCoord;

		uniform mat4 c_cameraToClipSpaceMatrix;

		void main() {
			vec4 pos = c_cameraToClipSpaceMatrix * vec4(c_position, 1);
			gl_Position = pos.xyww;
			texCoord = c_position;
		}

		#stage fragment

		out vec4 fragColor;
		in vec3 texCoord;
		
		uniform samplerCube c_mainTexture;
		uniform vec4 c_mainColor;

		void main() {
			fragColor = texture(c_mainTexture, texCoord) * c_mainColor;
		}

		ENDGLSL
	}
}