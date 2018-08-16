#ifndef _LIT_FRAGMENT_INCLUDED
#define _LIT_FRAGMENT_INCLUDED

in vec3 _ViewPos;
in vec4 _ShadowCoord;

uniform vec3 _SpecularColor;
uniform vec3 _EmissiveColor;
uniform float _Gloss;

uniform sampler2D _SSAOTexture;
uniform sampler2DShadow _ShadowDepthTexture;

vec2 _poissonDisk[] = vec2[]( 
   vec2(-0.94201624, -0.39906216), 
   vec2(0.94558609, -0.76890725), 
   vec2(-0.094184101, -0.92938870), 
   vec2(0.34495938, 0.29387760), 
   vec2(-0.91588581, 0.45771432), 
   vec2(-0.81544232, -0.87912464), 
   vec2(-0.38277543, 0.27676845), 
   vec2(0.97484398, 0.75648379), 
   vec2(0.44323325, -0.97511554), 
   vec2(0.53742981, -0.47373420), 
   vec2(-0.26496911, -0.41893023), 
   vec2(0.79197514, 0.19090188), 
   vec2(-0.24188840, 0.99706507), 
   vec2(-0.81409955, 0.91437590), 
   vec2(0.19984126, 0.78641367), 
   vec2(0.14383161, -0.14100790) 
);

float _random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dp = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dp) * 43758.5453);
}

vec3 _CalcDirectionalLight(vec3 worldPos, vec3 worldNormal, float visibility) {
	vec3 ambient = /*_AmbientColor * */texture(_SSAOTexture, _FragTexCoord()).rgb;

	float factor = clamp(dot(worldNormal, -_LightDir), 0, 1);
	vec3 diffuse = vec3(_LightColor * factor);

	vec3 toEye = normalize(_CameraPos - worldPos);
	vec3 reflectDir = normalize(reflect(_LightDir, worldNormal));
	factor = clamp(dot(toEye, reflectDir), 0, 1);
	factor = pow(factor, _Gloss);
	vec3 specular = vec3(_LightColor * factor) * _SpecularColor;

	return ambient; //+ _EmissiveColor + visibility * (diffuse + specular);
}

#define _SHADOW_LEVEL	 0
float _CalcShadowVisibility(vec3 worldPos) {
	float visibility = 1;
	float bias = 0.005;

	for (int i = 0; i < 4; ++i) {
	#if _SHADOW_LEVEL == 0
		// use either :
		//  - Always the same samples.
		//    Gives a fixed pattern in the shadow, but no noise
		int index = i;
	#elif _SHADOW_LEVEL == 1
		//  - A random sample, based on the pixel's screen location. 
		//    No banding, but the shadow moves with the camera, which looks weird.
		int index = int(16.0 * _random(gl_FragCoord.xyy, i)) % 16;
	#elif _SHADOW_LEVEL == 2
		//  - A random sample, based on the pixel's position in world space.
		//    The position is rounded to the millimeter to avoid too much aliasing
		int index = int(16.0 * _random(floor(worldPos.xyz * 1000.0), i)) % 16;
	#endif

		// being fully in the shadow will eat up 4*0.2 = 0.8
		// 0.2 potentially remain, which is quite dark.
		visibility -= 0.2 * (1 - texture(_ShadowDepthTexture, vec3(_ShadowCoord.xy + _poissonDisk[index] / 700.0,  (_ShadowCoord.z - bias)/_ShadowCoord.w)));
	}

	return visibility;
}

vec3 _ApplyFogColor(vec3 color) {
	float dist = length(_ViewPos);
	float factor = clamp(1.0 / exp(dist * _FogParams.w), 0, 1);
	return mix(_FogParams.xyz, color, factor);
}

#endif // _LIT_FRAGMENT_INCLUDED
