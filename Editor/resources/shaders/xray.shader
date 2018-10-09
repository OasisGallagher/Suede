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
		in vec3 _Pos;
		in vec3 _Normal;

		out vec3 viewDir;
		out vec3 normal;

		uniform mat4 _LocalToClipMatrix;
		uniform mat4 _LocalToWorldMatrix;

		void main() {
			// normal local to world space.
			normal = transpose(inverse(mat3(_LocalToWorldMatrix))) * _Normal;
			vec3 worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
			viewDir = _CameraPos - worldPos;
			
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
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
		in vec3 _Pos;
		in vec2 _TexCoord0;
		in vec3 _Normal;
		
		out vec2 texCoord;
		out vec3 worldPos;
		out vec3 normal;

		#include "builtin/include/lit_vertex.inc"

		uniform mat4 _LocalToClipMatrix;
		uniform mat4 _LocalToWorldMatrix;

		void main() {
			texCoord = _TexCoord0;

			normal = (_LocalToWorldMatrix * vec4(_Normal, 0)).xyz;
			worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
	
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

		#include "builtin/include/lit_fragment.inc"

		void main() {
			vec3 albedo = texture(_MainTexture, texCoord).xyz * _MainColor;
			float visibility = _CalcShadowVisibility(worldPos);
			fragColor = vec4(albedo, _CalcDirectionalLight(worldPos, normalize(normal), visibility), 1);
		}

		ENDGLSL
	 }
}
