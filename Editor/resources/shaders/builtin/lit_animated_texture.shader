Properties { }
SubShader {
	Pass {
		GLSLPROGRAM

		#include "builtin/include/suede.inc"
		
		#stage vertex

		in vec3 _Pos;
		in vec2 _TexCoord;
		in vec3 _Normal;

		in ivec4 _BoneIndexes;
		in vec4 _BoneWeights;

		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;

		#include "builtin/include/lit_vertex.inc"

		uniform mat4 _BoneToRootMatrices[_C_MAX_BONE_COUNT];

		void main() {
			mat4 mat = _BoneToRootMatrices[_BoneIndexes[0]] * _BoneWeights[0];
			mat += _BoneToRootMatrices[_BoneIndexes[1]] * _BoneWeights[1];
			mat += _BoneToRootMatrices[_BoneIndexes[2]] * _BoneWeights[2];
			mat += _BoneToRootMatrices[_BoneIndexes[3]] * _BoneWeights[3];

			texCoord = _TexCoord;

			normal = (_LocalToWorldMatrix * mat * vec4(_Normal, 0)).xyz;
			worldPos = (_LocalToWorldMatrix * mat * vec4(_Pos, 1)).xyz;

			_CALC_FOG_PARAMS();
			_CALC_SHADOW_COORD();

			gl_Position = _LocalToClipMatrix * mat * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;
		in mat4 o_matrix;

		uniform vec4 _MainColor;
		uniform sampler2D _MainTexture;

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
