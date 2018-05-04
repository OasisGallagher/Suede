Properties { }
SubShader {
	Pass {
		GLSLPROGRAM

		#include "builtin/include/suede.inc"
		
		#stage vertex

		in vec3 c_position;
		in vec2 c_texCoord;
		in vec3 c_normal;

		in ivec4 c_boneIndexes;
		in vec4 c_boneWeights;

		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;

		#include "builtin/include/lit_vertex.inc"

		uniform mat4 c_boneToRootMatrices[C_MAX_BONE_COUNT];

		void main() {
			mat4 mat = c_boneToRootMatrices[c_boneIndexes[0]] * c_boneWeights[0];
			mat += c_boneToRootMatrices[c_boneIndexes[1]] * c_boneWeights[1];
			mat += c_boneToRootMatrices[c_boneIndexes[2]] * c_boneWeights[2];
			mat += c_boneToRootMatrices[c_boneIndexes[3]] * c_boneWeights[3];

			texCoord = c_texCoord;

			normal = (c_localToWorldMatrix * mat * vec4(c_normal, 0)).xyz;
			worldPos = (c_localToWorldMatrix * mat * vec4(c_position, 1)).xyz;

			//calculateShadowCoord();

			gl_Position = c_localToClipMatrix * mat * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;
		in mat4 o_matrix;

		uniform vec4 c_mainColor;
		uniform sampler2D c_mainTexture;

		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec4 albedo = texture(c_mainTexture, texCoord) * c_mainColor;
			float visibility = 1; //calculateShadowVisibility();
			fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal), visibility), 1);
		}
		
		ENDGLSL
	}
}
