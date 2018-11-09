Properties {
	color _NormalColor = { 0, 1, 0 };
	franged _NormalLength = { 0.4, 0.1, 2 };
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
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;
		
		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_vertex.inc"

		void main() {
			texCoord = _TexCoord0;

			normal = transpose(inverse(mat3(_LocalToWorldMatrix))) * _Normal;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
			
			_CALC_FOG_PARAMS();
			_CALC_SHADOW_COORD();

			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage fragment
		out vec4 fragColor;
		
		in vec2 texCoord;
		in vec3 worldPos;
		in vec3 normal;

		uniform vec4 _MainColor;
		uniform sampler2D _MainTexture;

		#include "builtin/include/suede.inc"
		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec4 albedo = texture(_MainTexture, texCoord) * _MainColor;
			float visibility = _CalcShadowVisibility(worldPos);
			fragColor = vec4(_CalcDirectionalLight(albedo.xyz, worldPos, normalize(normal), visibility), albedo.a);
			fragColor.xyz = _ApplyFogColor(fragColor.xyz);
		}

		ENDGLSL
	}

	Pass {
		ZTest Off;
		Blend Off;

		GLSLPROGRAM

		#stage vertex
		in vec3 _Pos;
		in vec3 _Normal;
		
		out VS_OUT{
			vec3 clipNormal;
		};
		
		#include "builtin/include/suede.inc"

		void main() {
			clipNormal = normalize((_LocalToClipMatrix * vec4(_Normal, 0)).xyz);
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
		}

		#stage geometry
		#include "builtin/include/suede.inc"

		layout (triangles) in;
		layout (line_strip, max_vertices = 6) out;
		
		in VS_OUT {
			vec3 clipNormal;
		} inputs[];

		uniform float _NormalLength;

		void main() {
			for (int i = 0; i < 3; ++i) {
				gl_Position = gl_in[i].gl_Position;
				EmitVertex();

				gl_Position = gl_in[i].gl_Position + vec4(inputs[i].clipNormal, 0) * _NormalLength;
				EmitVertex();

				EndPrimitive();
			}
		}

		#stage fragment
		out vec4 fragColor;

		#include "builtin/include/suede.inc"
		
		uniform vec3 _NormalColor;
		void main() {
			fragColor = vec4(_NormalColor, 1);
		}

		ENDGLSL
	}
}
