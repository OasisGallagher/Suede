#pragma once
#define VARIABLE_PREFIX					"c_"
#define VARIABLE_PREFIX_LENGTH			2

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

	DEFINE_VARIABLE(instanceColor);
	DEFINE_VARIABLE(instanceGeometry);

	DEFINE_VARIABLE(boneToRootMatrices);

	DEFINE_VARIABLE(mainTexture);
	DEFINE_VARIABLE(bumpTexture);
	DEFINE_VARIABLE(specularTexture);
	DEFINE_VARIABLE(emissiveTexture);
	DEFINE_VARIABLE(lightmapTexture);

	DEFINE_VARIABLE(shadowDepthTexture);

	DEFINE_VARIABLE(gloss);
	DEFINE_VARIABLE(mainColor);
	DEFINE_VARIABLE(specularColor);
	DEFINE_VARIABLE(emissiveColor);

	DEFINE_VARIABLE(ambientLightColor);

	DEFINE_VARIABLE(lightColor);
	DEFINE_VARIABLE(lightPosition);
	DEFINE_VARIABLE(lightDirection);

	DEFINE_VARIABLE(cameraPosition);

	DEFINE_VARIABLE(decalMatrix);
	DEFINE_VARIABLE(worldToCameraMatrix);
	DEFINE_VARIABLE(localToClipMatrix);
	DEFINE_VARIABLE(localToWorldMatrix);
	DEFINE_VARIABLE(worldToOrthographicLightMatrix);
	DEFINE_VARIABLE(worldToClipMatrix);
	DEFINE_VARIABLE(cameraToClipMatrix);
};

#undef DEFINE_VARIABLE
