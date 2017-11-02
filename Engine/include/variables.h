#pragma once
#define VARIABLE_PREFIX					"c_"
#define VARIABLE_NAME(name)				(VARIABLE_PREFIX #name)
#define DEFINE_VARIABLE(name)			static const char* name = VARIABLE_NAME(name)

#define C_MAX_BONE_COUNT				128

namespace Variables {
	DEFINE_VARIABLE(time);
	DEFINE_VARIABLE(deltaTime);

	DEFINE_VARIABLE(position);
	DEFINE_VARIABLE(texCoord);
	DEFINE_VARIABLE(normal);
	DEFINE_VARIABLE(tangent);
	DEFINE_VARIABLE(boneIndexes);
	DEFINE_VARIABLE(boneWeights);

	DEFINE_VARIABLE(user0);
	DEFINE_VARIABLE(user1);
	DEFINE_VARIABLE(user2);

	DEFINE_VARIABLE(boneToRootSpaceMatrices);

	DEFINE_VARIABLE(fragColor);
	DEFINE_VARIABLE(depth);

	DEFINE_VARIABLE(mainTexture);
	DEFINE_VARIABLE(bumpTexture);
	DEFINE_VARIABLE(specularTexture);

	DEFINE_VARIABLE(shadowDepthTexture);

	DEFINE_VARIABLE(gloss);

	DEFINE_VARIABLE(ambientLightColor);

	DEFINE_VARIABLE(lightColor);
	DEFINE_VARIABLE(lightPosition);
	DEFINE_VARIABLE(lightDirection);

	DEFINE_VARIABLE(cameraPosition);

	DEFINE_VARIABLE(worldToCameraSpaceMatrix);
	DEFINE_VARIABLE(localToClipSpaceMatrix);
	DEFINE_VARIABLE(localToWorldSpaceMatrix);
	DEFINE_VARIABLE(localToShadowSpaceMatrix);
	DEFINE_VARIABLE(localToOrthographicLightSpaceMatrix);
	DEFINE_VARIABLE(worldToClipSpaceMatrix);
};

#undef DEFINE_VARIABLE
