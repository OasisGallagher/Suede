Properties {
	float outlineFactor = 0.1;
	color3 outlineColor = { 0.98, 0.51, 0.04 };
}

SubShader {
	Tags { 
		Queue = "Transparent";
	}
	
	Pass {
		//ZWrite On;
		ZTest LEqual;
		Blend SrcAlpha OneMinusSrcAlpha;

		//StencilTest Always 1 0xFF;
		//StencilOp Keep Keep Replace;
		//StencilWrite On;

		GLSLPROGRAM

		#stage vertex
		in vec3 _Pos;
		in vec2 _TexCoord;
		in vec3 _Normal;
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;
		
		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_vertex.inc"

		void main() {
			texCoord = _TexCoord;

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

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec4 albedo = texture(_MainTexture, texCoord) * _MainColor;
			float visibility = _CalcShadowVisibility(worldPos);
			fragColor = albedo * vec4(_CalcDirectionalLight(worldPos, normalize(normal), visibility), 1);
			fragColor.xyz = _ApplyFogColor(fragColor.xyz);
		}

		ENDGLSL
	}
}
