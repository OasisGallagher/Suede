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
		ZTest Off;
		StencilTest NotEqual 1 0xFF;
		StencilMask FrontAndBack 0x00;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		in vec3 c_position;
		in vec3 c_normal;

		uniform mat4 c_localToClipSpaceMatrix;

		void main() {
			// normal local to world space.
			vec3 position = c_position + c_normal * 0;
			float scale = 1.1;
			mat4 scaleMatrix = mat4(
				scale, 0, 0, 0,
				0, scale, 0, 0,
				0, 0, scale, 0,
				0, 0, 0, 1		
			);

			gl_Position = c_localToClipSpaceMatrix * scaleMatrix * vec4(position, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		void main() {
			fragColor = vec4(1, 0, 0, 0.5);
		}

		ENDGLSL
	}
}
