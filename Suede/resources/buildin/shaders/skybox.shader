Properties {
}

SubShader {
	Pass {
		Cull Front;
		ZTest LEqual;

		GLSLPROGRAM

		#stage vertex

		in vec3 c_position;
		out vec3 texCoord;

		uniform mat4 c_cameraToClipSpaceMatrix;
		uniform mat4 c_localToWorldSpaceMatrix;
		uniform mat4 c_worldToCameraSpaceMatrix;

		void main() {
			vec4 pos = c_cameraToClipSpaceMatrix * vec4(mat3(c_worldToCameraSpaceMatrix * c_localToWorldSpaceMatrix) * c_position, 1);
			gl_Position = pos.xyww;
			texCoord = c_position;
		}

		#stage fragment

		out vec4 color;
		in vec3 texCoord;
		
		uniform samplerCube c_mainTexture;
		uniform vec4 c_mainColor;

		void main() {
			color = texture(c_mainTexture, texCoord) * c_mainColor;
		}

		ENDGLSL
	}
}