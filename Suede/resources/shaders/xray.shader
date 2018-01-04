Properties {
	// Rim color.
	vec4 xrayColor = { 0.2f, 0.55f, 0.73f, 1 };
}

SubShader {
	Tags { Queue = "Geometry + 200"; }

	Pass {
		ZWrite Off;
		ZTest Greater;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;
		in vec3 c_cameraPosition;
		in vec3 c_normal;

		out vec3 viewDir;
		out vec3 normal;

		uniform mat4 c_localToClipSpaceMatrix;
		uniform mat4 c_localToWorldSpaceMatrix;

		void main() {
			// normal local to world space.
			normal = transpose(inverse(mat3(c_localToWorldSpaceMatrix))) * c_normal;
			vec3 worldPos = (c_localToWorldSpaceMatrix * vec4(c_position, 1)).xyz;
			viewDir = c_cameraPosition - worldPos;
			
			gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		in vec3 normal;
		in vec3 viewDir;

		uniform vec4 xrayColor;

		void main() {
			float factor = 1 - dot(normalize(normal), normalize(viewDir));
			fragColor = xrayColor * factor;
		}

		ENDGLSL
	}

	Pass {
		ZTest LEqual;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;
		in vec2 c_texCoord;
		in vec3 c_normal;
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;

		#include "buildin/shaders/include/light_vertex.inc"

		uniform mat4 c_localToClipSpaceMatrix;
		uniform mat4 c_localToWorldSpaceMatrix;

		void main() {
			texCoord = c_texCoord;

			normal = (c_localToWorldSpaceMatrix * vec4(c_normal, 0)).xyz;
			worldPos = (c_localToWorldSpaceMatrix * vec4(c_position, 1)).xyz;
	
			calculateShadowCoord();

			gl_Position = c_localToClipSpaceMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;

		uniform vec4 c_mainColor;
		uniform sampler2D c_mainTexture;

		#include "buildin/shaders/include/light_fragment.inc"

		void main() {
			vec4 albedo = texture(c_mainTexture, texCoord) * c_mainColor;
			float visibility = calculateShadowVisibility();
			fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal), visibility), 1);
		}

		ENDGLSL
	 }
}
