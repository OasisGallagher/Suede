Properties {
	color _MainColor = { 1, 1, 1, 1 };
	frange _HeightScale = { 0.1, 0.1, 2 };

	texture2D _MainTexture = "white";
	texture2D _BumpTexture = "white";
	texture2D _DepthTexture = "white";
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

		out mat3 worldToTangentMatrix;

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_vertex.inc"

		void main() {
			texCoord = _TexCoord0;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;

			_CALCULATE_FOG_PARAMS();
			_CALCULATE_SHADOW_COORD();

			_TANGENT_TO_WORLD_ROTATION(worldToTangentMatrix);
			worldToTangentMatrix = transpose(worldToTangentMatrix);

			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		in vec3 worldPos;
		in mat3 worldToTangentMatrix;

		uniform float _HeightScale;
		uniform sampler2D _MainTexture;
		uniform sampler2D _BumpTexture;
		uniform sampler2D _DepthTexture;

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_fragment.inc"

		vec2 parallaxMapping(vec2 texCoord, vec3 viewDir) {
			float height = texture(_DepthTexture, texCoord).r;
			return texCoord - viewDir.xy * height * _HeightScale;
		}

		void main() {
			vec3 viewDir = normalize(worldToTangentMatrix * (_CameraPos - worldPos));
			vec2 newTexCoord = parallaxMapping(texCoord, viewDir);
			vec3 normal = texture(_BumpTexture, newTexCoord).xyz * 2 - 1;
			normal = normalize(transpose(worldToTangentMatrix) * normal);

			vec4 albedo = texture(_MainTexture, newTexCoord);
			float visibility = _CalculateShadowVisibility(worldPos);
			fragColor = vec4(_CalculateDirectionalLight(albedo.xyz, worldPos, normal, visibility), albedo.a);
			fragColor.xyz = _ApplyFogColor(fragColor.xyz);
		}

		ENDGLSL
	}
}

