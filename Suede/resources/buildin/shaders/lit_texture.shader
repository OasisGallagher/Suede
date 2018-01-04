Properties {
}

SubShader {
	Pass {
		ZTest Less;

		StencilTest Always 1 0xFF;
		StencilOp Keep Keep Replace;
		StencilMask FrontAndBack 0xFF;

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

	Pass {
		ZWrite Off;
		ZTest Always;
		StencilTest NotEqual 1 0xFF;
		StencilMask FrontAndBack 0x00;

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
			fragColor.a = 0;
		}

		ENDGLSL
	}
}
