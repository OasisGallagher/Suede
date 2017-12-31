Properties { }

SubShader {
	Pass {
		Cull Back;
		DepthWrite On;
		DepthTest LessEqual;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;

		uniform mat4 c_localToOrthographicLightSpaceMatrix;

		void main() {
			gl_Position = c_localToOrthographicLightSpaceMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out float depth;

		void main() {
			depth = gl_FragCoord.z;
		}

		ENDGLSL
	}
}