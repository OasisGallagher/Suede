Properties {
	color4 outlineColor = { 0.98, 0.51, 0.04, 0.5 };
}

SubShader {
	Pass {
		ZTest LEqual;
		Blend SrcAlpha OneMinusSrcAlpha;

		StencilTest Always 1 0xFF;
		StencilOp Keep Keep Replace;
		StencilWrite On;

		GLSLPROGRAM
		
		#include "builtin/include/suede.inc"

		#stage vertex
		in vec3 _Pos;
		in vec2 _TexCoord;
		in vec3 _Normal;
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;

		#include "builtin/include/lit_vertex.inc"

		void main() {
			texCoord = _TexCoord;

			normal = transpose(inverse(mat3(_LocalToWorldMatrix))) * _Normal;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
	
			calculateShadowCoord();

			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;

		uniform vec4 _MainColor;
		uniform sampler2D _MainTexture;

		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec4 albedo = texture(_MainTexture, texCoord) * _MainColor;
			float visibility = calculateShadowVisibility();
			fragColor = albedo * vec4(calculateDirectionalLight(worldPos, normalize(normal), visibility), 1);
			//fragColor = texelFetch(_MatrixBuffer, _MatrixBufferOffset + 0 * 8);
			//fragColor.a = 1;
			//fragColor = vec4(visibility, visibility, visibility, 1);
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
		#include "builtin/include/suede.inc"

		in vec3 _Pos;
		in vec3 _Normal;

		void main() {
			mat3 m3 = transpose(inverse(mat3(_LocalToWorldMatrix)));
			vec3 normal = m3 * _Normal;
			normal.xy = mat2(_WorldToClipMatrix) * normal.xy;
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
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
