Properties { }

SubShader {
	Pass {
		Cull Back;
		DepthWrite On;
		DepthTest LessEqual;

		GLSLPROGRAM
		#stage vertex
		in vec3 c_position;

		uniform mat4 c_localToClipSpaceMatrix;

		void main() {
			gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out float depth;

		void main() {
			depth = gl_FragCoord.z;
		}

		ENDGLSL
	}
}