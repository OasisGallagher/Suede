Properties { }

SubShader {
	Pass {
		Cull Back;
		ZWrite On;
		ZTest LEqual;

		GLSLPROGRAM
		#stage vertex
		in vec3 c_position;

		uniform mat4 c_localToClipMatrix;

		void main() {
			gl_Position = c_localToClipMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out float depth;

		void main() {
			depth = gl_FragCoord.z;
		}

		ENDGLSL
	}
}