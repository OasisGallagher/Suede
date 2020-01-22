Properties {
	color _MainColor = { 1, 1, 1, 1 };
	texture2D _MainTexture = "white";
}

SubShader {
	Tags { 
		Queue = "Transparent";
	}
	
	Pass {
		ZTest LEqual;
		ZWrite On;
		Blend SrcAlpha OneMinusSrcAlpha;

		StencilTest Always 1 0xFF;
		StencilOp Keep Keep Replace;
		StencilWrite On;

		GLSLPROGRAM

		#stage vertex
		in vec3 _Pos;
		in vec2 _TexCoord0;
		in vec3 _Normal;
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;
		
		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_vertex.inc"

		void main() {
			texCoord = _TexCoord0;

			normal = transpose(inverse(mat3(_LocalToWorldMatrix))) * _Normal;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
			
			_CALC_FOG_PARAMS();
			_CALC_SHADOW_COORD();

			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;

		uniform vec4 _MainColor;
		uniform sampler2D _MainTexture;
		uniform sampler2D _CameraDepthTexture;

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec4 albedo = vec4(texture(_MainTexture, texCoord).rgb, 1) * _MainColor;
			float visibility = _CalcShadowVisibility(worldPos);
			fragColor = vec4(_CalcDirectionalLight(albedo.xyz, worldPos, normalize(normal), visibility), albedo.a);
			fragColor.xyz = _ApplyFogColor(fragColor.xyz);
		}

		ENDGLSL
	}
}
