Properties { }
SubShader {
	GLSLPROGRAM
		#stage vertex
		in vec3 _Pos;
		in vec2 _TexCoord;
		in vec3 _Normal;
		in vec3 _Tangent;

		out vec2 texCoord;
		out vec3 worldPos;
		out mat3 tangentToWorldMatrix;

		uniform mat4 _LocalToClipMatrix;
		uniform mat4 _LocalToWorldMatrix;

		void main() {
			texCoord = _TexCoord;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;

			vec3 worldNormal = (_LocalToWorldMatrix * vec4(_Normal, 0)).xyz;
			vec3 worldTangent = (_LocalToWorldMatrix * vec4(_Tangent, 0)).xyz;
			vec3 worldBitangent = cross(worldNormal, worldTangent);
			
			vec3 bitangent = cross(_Normal, _Tangent);
			tangentToWorldMatrix = mat3(worldTangent, worldBitangent, worldNormal);

			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		in vec3 worldPos;
		in mat3 tangentToWorldMatrix;

		uniform sampler2D _MainTexture;
		uniform sampler2D _BumpTexture;

		#include "builtin/include/light.inc"

		void main() {
			vec3 normal = texture(_BumpTexture, texCoord).xyz;
			normal = tangentToWorldMatrix * normal;

			vec4 albedo = texture(_MainTexture, texCoord);
			fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal)), 1);
		}
		
		ENDGLSL
	}
}

