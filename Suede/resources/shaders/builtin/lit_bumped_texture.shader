Properties { }
SubShader {
	GLSLPROGRAM
		#stage vertex
		in vec3 c_position;
		in vec2 c_texCoord;
		in vec3 c_normal;
		in vec3 c_tangent;

		out vec2 texCoord;
		out vec3 worldPos;
		out mat3 tangentToWorldMatrix;

		uniform mat4 c_localToClipMatrix;
		uniform mat4 c_localToWorldMatrix;

		void main() {
			texCoord = c_texCoord;
			worldPos = (c_localToWorldMatrix * vec4(c_position, 1)).xyz;

			vec3 worldNormal = (c_localToWorldMatrix * vec4(c_normal, 0)).xyz;
			vec3 worldTangent = (c_localToWorldMatrix * vec4(c_tangent, 0)).xyz;
			vec3 worldBitangent = cross(worldNormal, worldTangent);
			
			vec3 bitangent = cross(c_normal, c_tangent);
			tangentToWorldMatrix = mat3(worldTangent, worldBitangent, worldNormal);

			gl_Position = c_localToClipMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;

		in vec2 texCoord;
		in vec3 worldPos;
		in mat3 tangentToWorldMatrix;

		uniform sampler2D c_mainTexture;
		uniform sampler2D c_bumpTexture;

		#include "builtin/include/light.inc"

		void main() {
			vec3 normal = texture(c_bumpTexture, texCoord).xyz;
			normal = tangentToWorldMatrix * normal;

			vec4 albedo = texture(c_mainTexture, texCoord);
			fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal)), 1);
		}
		
		ENDGLSL
	}
}

