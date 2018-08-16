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

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		in vec2 _TexCoord;

		out vec2 viewRay;
		out vec2 texCoord;

		void main() {
			texCoord = _TexCoord;
			viewRay = vec2(_Pos.x * _ProjParams.z * _ProjParams.w, _Pos.y * _ProjParams.w);
			gl_Position = vec4(_Pos, 1);
		}

		#stage fragment
		#include "builtin/include/suede.inc"

		in vec2 viewRay;
		in vec2 texCoord;

		out vec3 fragColor;

		uniform float sampleRadius; 
		uniform sampler2D _MainTexture;
		uniform vec3 _SSAOKernel[_C_SSAO_KERNEL_SIZE];

		vec3 viewSpacePos(vec2 coord) {
			float z = _LinearEyeDepth(texture(_MainTexture, coord).x);
			return vec3(viewRay * z, z);
		}

		void main() {
			if (texture(_MainTexture, texCoord).x == 1) {
				fragColor = vec3(0);
				return;
			}

			float occlusion = 0;
			vec3 pos = viewSpacePos(texCoord);

			for (int i = 0; i < _C_SSAO_KERNEL_SIZE; ++i) {
				vec3 samplePos = pos + _SSAOKernel[i];
				vec4 offset = _CameraToClipMatrix * vec4(samplePos, 1);
				offset.xy = offset.xy * 0.5 / offset.w + 0.5;

				float sampleDepth = viewSpacePos(offset.xy).z;
				//float f = smoothstep(0, 1, sampleRadius / abs(pos.z - sampleDepth));
				if (abs(pos.z - sampleDepth) < sampleRadius) {
					occlusion += step(sampleDepth, samplePos.z);
				}
			}

			occlusion = 1.0 - occlusion / _C_SSAO_KERNEL_SIZE;
			fragColor = vec3(occlusion * occlusion);
		}

		ENDGLSL
	}

	Pass {
		Cull Off;
		Blend Off;
		ZWrite Off;
		ZTest Off;
		
		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/post_effect_vertex.inc"

		#stage fragment
		#include "builtin/include/suede.inc"

		in vec2 texCoord;
		out vec3 fragColor;

		uniform sampler2D _MainTexture;
		const float offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

		void main()
		{
			vec3 color = vec3(0);
			vec2 size = 1.0 / textureSize(_MainTexture, 0);

			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					vec2 tc = vec2(texCoord.x + offsets[j] * size.x, texCoord.y + offsets[i] * size.y);
					color += texture(_MainTexture, tc).rgb;
				}
			}

			color /= 16.0;
			fragColor = color;
		}

		ENDGLSL
	}
}
