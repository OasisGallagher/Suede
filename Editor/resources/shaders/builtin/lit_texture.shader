Properties {
	franged _RimFactor = { 0.1, 0.01, 0.5 };
	color _RimColor = { 0.98, 0.51, 0.04 };
	color _MainColor = { 1, 1, 1, 1 };
	texture2D _MainTexture = "white";
}

SubShader {
	Tags { 
		Queue = "Transparent";
	}

	Pass "Outline" false {
		Cull Front;
		ZWrite Off;
		Offset 100 100;

		//StencilTest NotEqual 1 0xFF;
		//StencilWrite Off;

		GLSLPROGRAM

		#stage vertex
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		in vec3 _Normal;

		uniform float _RimFactor;
		void main() {
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);

			vec3 normal = transpose(inverse(mat3(_WorldToCameraMatrix * _LocalToWorldMatrix))) * _Normal;
			vec2 offset = mat2(_CameraToClipMatrix) * normal.xy;
			gl_Position.xy += offset * _RimFactor;
		}

		#stage fragment
		out vec4 fragColor;
		
		uniform vec3 _RimColor;
		void main() {
			fragColor = vec4(_RimColor, 1);
		}

		ENDGLSL
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

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec4 albedo = texture(_MainTexture, texCoord) * _MainColor;
			float visibility = _CalcShadowVisibility(worldPos);
			fragColor = vec4(_CalcDirectionalLight(albedo.xyz, worldPos, normalize(normal), visibility), albedo.a);
			fragColor.xyz = _ApplyFogColor(fragColor.xyz);
		}

		ENDGLSL
	}
}
