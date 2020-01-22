// https://github.com/candycat1992/Unity_Shaders_Book/blob/master/Assets/Shaders/Chapter13/Chapter13-EdgeDetectNormalAndDepth.shader

Properties {
	float _SampleDistance = 1;
	color _OutlineColor = { 0.98, 0.51, 0.04, 1 };
}

SubShader {
	Pass {
		Cull Off;
		ZWrite Off;
		ZTest Off;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 _Pos;
		in vec2 _TexCoord0;

		out vec2 texCoords[5];
		
		uniform float _SampleDistance;
		uniform sampler2D _MainTexture;

		void main() {
			texCoords[0] = _TexCoord0;
			
			vec2 size = 1.0 / textureSize(_MainTexture, 0);
			texCoords[1] = _TexCoord0 + size * vec2(1, 1) * _SampleDistance;
			texCoords[2] = _TexCoord0 + size * vec2(-1, -1) * _SampleDistance;
			texCoords[3] = _TexCoord0 + size * vec2(-1, 1) * _SampleDistance;
			texCoords[4] = _TexCoord0 + size * vec2(1, -1) * _SampleDistance;

			gl_Position = vec4(_Pos, 1);
		}

		#stage fragment

		out vec4 fragColor;

		in vec2 texCoords[5];

		uniform vec4 _OutlineColor;
		uniform sampler2D _MainTexture;
		uniform sampler2D _CameraDepthTexture;
		
		bool isEdge(float centerDepth, float sampleDepth) {
			return (centerDepth >= 1 && sampleDepth < 1) || (centerDepth < 1 && sampleDepth >= 1);
		}
		
		void main() {
			bool edge = isEdge(texture(_CameraDepthTexture, texCoords[1]).x, texture(_CameraDepthTexture, texCoords[2]).x) 
				|| isEdge(texture(_CameraDepthTexture, texCoords[3]).x, texture(_CameraDepthTexture, texCoords[4]).x);

			fragColor = mix(vec4(0, 0, 0, 1), _OutlineColor, float(edge));
			//fragColor = texture(_MainTexture, texCoords[0]);
			//fragColor = vec4(texture(_CameraDepthTexture, texCoords[0]).xyz, 1);
		}

		ENDGLSL
	}
}
