Properties { 
	color _MainColor = { 1, 1, 1, 1 };
	texture2D _MainTexture = "white";
}

SubShader {
	Pass {
		ZTest LEqual;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM
		#stage vertex
		in vec3 _Pos;
		in vec2 _TexCoord0;
		in vec3 _Normal;
		in vec3 _Tangent;

		out vec2 texCoord;
		out vec3 worldPos;
		out mat3 tangentToWorldMatrix;

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_vertex.inc"

		void main() {
			texCoord = _TexCoord0;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;

			_CALC_FOG_PARAMS();
			_CALC_SHADOW_COORD();

			_TANGENT_SPACE_ROTATION(tangentToWorldMatrix);

			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		in vec3 worldPos;
		in mat3 tangentToWorldMatrix;

		uniform sampler2D _MainTexture;
		uniform sampler2D _BumpTexture;

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec3 normal = texture(_BumpTexture, texCoord).xyz;
			normal = normalize(tangentToWorldMatrix * normal);

			vec4 albedo = texture(_MainTexture, texCoord);
			float visibility = _CalcShadowVisibility(worldPos);
			fragColor = vec4(_CalcDirectionalLight(albedo.xyz, worldPos, normal, visibility), albedo.a);
			fragColor.xyz = _ApplyFogColor(fragColor.xyz);
		}
		
		ENDGLSL
	}
}
