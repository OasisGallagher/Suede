Properties {
	int x = 4;
	tex2 main = { 255, 0, 0 };
}

SubShader {
	Tags {
		Queue = "opaque";
		Queue = "geometry";
		Queue = "transparent";
	}
	
	Pass {
		Cull Off;
		
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

SubShader {
	Tags {
		Queue = "opaque";
		Queue = "geometry";
		Queue = "transparent";
	}
	
	Pass {
		Cull Off;
		Blend SrcAlpha OneMinusSrcAlpha;
		
		GLSLPROGRAM

		#include "main.inc"

		ENDGLSL
	}
}
