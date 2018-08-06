Properties { }

SubShader {
	Pass {
		Offset 100 100;
		Cull Back;
		ZWrite On;
		ZTest LEqual;
		//Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		
		out vec4 projTexCoord;
		out vec4 clipPosition;

		uniform mat4 _DecalMatrix;

		void main() {
			projTexCoord = _DecalMatrix * vec4(_Pos, 1);
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
			clipPosition = gl_Position;
		}

		#stage fragment
		out vec4 fragColor;

		in vec4 clipPosition;
		in vec4 projTexCoord;
		uniform sampler2D _MainTexture;

		void main() {
			fragColor = textureProj(_MainTexture, projTexCoord);
		}

		ENDGLSL
	}
}
