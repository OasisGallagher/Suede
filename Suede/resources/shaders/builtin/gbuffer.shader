Properties { }

SubShader {
	Pass {
		Cull Back;
		Blend Off;
		ZWrite On;
		ZTest LEqual;

		GLSLPROGRAM

		#stage vertex

		in vec3 _Pos;
		in vec2 _TexCoord;
		in vec3 _Normal;

		uniform mat4 _LocalToClipMatrix;
		uniform mat4 _LocalToWorldMatrix;

		out VertOut {
			out	vec3 worldPos;
			out	vec2 texCoord;
			out	vec3 worldNormal;
		} vo;

		void main() {
			gl_Position = _LocalToClipMatrix * vec4(_Pos, 1);
			vo.worldPos = (_LocalToWorldMatrix * vec4(_Pos, 1)).xyz;
			vo.texCoord = _TexCoord;

			mat3 m = inverse(transpose(mat3(_LocalToWorldMatrix)));
			vo.worldNormal = m * _Normal;
		}

		#stage fragment

		in VertOut {
			in	vec3 worldPos;
			in	vec2 texCoord;
			in	vec3 worldNormal;
		} fi;

		out vec3 worldPos;
		out vec3 albedo;
		out vec3 worldNormal;

		uniform sampler2D _MainTexture;

		void main() {
			worldPos = fi.worldPos;
			albedo = texture(_MainTexture, fi.texCoord).rgb;
			worldNormal = normalize(fi.worldNormal);
		}

		ENDGL
	}
}
