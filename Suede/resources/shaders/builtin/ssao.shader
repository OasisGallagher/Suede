Properties { 
	float sampleRadius = 1.5;
}

SubShader {
	Pass {
		Cull Off;
		Blend Off;
		ZWrite Off;
		ZTest Off;

		GLSLPROGRAM
		
		#include "builtin/include/suede.inc"

		#stage vertex

		in vec3 _Pos;

		out vec2 viewRay;
		out vec2 texCoord;

		void main() {
			texCoord = (_Pos.xy + 1) / 2;
			viewRay = vec2(_Pos.x * _ProjParams.z * _ProjParams.w, _Pos.y * _ProjParams.w);
			gl_Position = vec4(_Pos, 1);
		}

		#stage fragment
		in vec2 viewRay;
		in vec2 texCoord;

		out vec3 fragColor;

		uniform float sampleRadius; 
		uniform vec3 _SSAOKernel[_C_SSAO_KERNAL_SIZE];

		vec3 viewSpacePos(vec2 coord) {
			float z = _LinearEyeDepth(texture(_DepthTexture, coord).x);
			return vec3(viewRay * z, z);
		}

		void main() {
			float AO = 0;
			vec3 pos = viewSpacePos(texCoord);

			for (int i = 0; i < _C_SSAO_KERNAL_SIZE; ++i) {
				vec3 samplePos = pos + _SSAOKernel[i];
				vec4 offset = _CameraToClipMatrix * vec4(samplePos, 1);
				offset.xy = offset.xy * 0.5 / offset.w + 0.5;

				float sampleDepth = viewSpacePos(offset.xy).x;
				if (abs(pos.z - sampleDepth) < sampleRadius) {
					AO += step(sampleDepth, samplePos.z);
				}
			}

			AO = 1.0 - AO / (_C_SSAO_KERNAL_SIZE / 2.0);
			fragColor = vec3(pow(AO, 2)) * 100000;
		}

		ENDGLSL
	}
}
