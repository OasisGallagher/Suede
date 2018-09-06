Properties { }

SubShader {
	Tags { Queue = "Transparent"; }

	Pass {
		ZWrite Off;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 _Pos;
		in vec4 _InstanceColor;
		in vec4 _InstanceGeometry;
		
		out vec2 texCoord;
		out vec4 color;

		#include "builtin/include/suede.inc"

		void main() {
			float size = _InstanceGeometry.w;
			vec3 center = _InstanceGeometry.xyz;

			vec3 cameraUp = vec3(_WorldToCameraMatrix[0][1], _WorldToCameraMatrix[1][1], _WorldToCameraMatrix[2][1]);
			vec3 cameraRight = vec3(_WorldToCameraMatrix[0][0], _WorldToCameraMatrix[1][0], _WorldToCameraMatrix[2][0]);

			vec3 position = center + cameraUp * _Pos.y * size + cameraRight * _Pos.x * size;
			gl_Position = _WorldToClipMatrix * vec4(position, 1);

			texCoord = _Pos.xy + vec2(0.5);
			color = _InstanceColor;
		}

		#stage fragment

		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec4 color;

		uniform sampler2D _MainTexture;

		void main() {
			fragColor = texture(_MainTexture, texCoord) * color; 
		}

		ENDGLSL
	}
}
