Properties {
	color4 outlineColor = { 0.98, 0.51, 0.04, 0.5 };
}

SubShader {
	Pass {
		ZTest LEqual;
		//Blend SrcAlpha OneMinusSrcAlpha;

		StencilTest Always 1 0xFF;
		StencilOp Keep Keep Replace;
		StencilWrite On;

		GLSLPROGRAM
		
		#include "buildin/include/suede.inc"

		#stage vertex
		in vec3 c_position;
		in vec2 c_texCoord;
		in vec3 c_normal;
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;

		#include "buildin/include/light_vertex.inc"

		void main() {
			texCoord = c_texCoord;

			normal = transpose(inverse(mat3(c_localToWorldMatrix))) * c_normal;
			worldPos = (c_localToWorldMatrix * vec4(c_position, 1)).xyz;
	
			calculateShadowCoord();

			gl_Position = c_localToClipMatrix * vec4(c_position, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;

		uniform vec4 c_mainColor;
		uniform sampler2D c_mainTexture;

		#include "buildin/include/light_fragment.inc"

		void main() {
			vec4 albedo = texture(c_mainTexture, texCoord) * c_mainColor;
			float visibility = calculateShadowVisibility();
			fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal), visibility), 1);
		}

		ENDGLSL
	}

	Pass "Outline" false {
		ZWrite Off;
		ZTest Off;
		//Cull Front;
		StencilTest NotEqual 1 0xFF;
		StencilWrite Off;
		Blend SrcAlpha OneMinusSrcAlpha;

		GLSLPROGRAM

		#stage vertex
		#include "buildin/include/suede.inc"

		in vec3 c_position;
		in vec3 c_normal;

		void main() {
			mat3 m3 = transpose(inverse(mat3(c_localToWorldMatrix)));
			vec3 normal = m3 * c_normal;
			normal.xy = mat2(c_worldToClipMatrix) * normal.xy;
			gl_Position = c_localToClipMatrix * vec4(c_position, 1);
			gl_Position.xy += normal.xy * 0.4;
		}

		#stage fragment
		out vec4 fragColor;
		
		uniform vec4 outlineColor;
		void main() {
			fragColor = outlineColor;
		}

		ENDGLSL
	}
}
