#pragma once
#define VARIABLE_PREFIX					"_"

#define VARIABLE_NAME(name)				(VARIABLE_PREFIX #name)
#define DEFINE_VARIABLE(name)			static const char* name = VARIABLE_NAME(name)

namespace Variables {
	DEFINE_VARIABLE(Pos);
	DEFINE_VARIABLE(TexCoord);
	DEFINE_VARIABLE(Normal);
	DEFINE_VARIABLE(Tangent);
	DEFINE_VARIABLE(BoneIndexes);
	DEFINE_VARIABLE(BoneWeights);

	DEFINE_VARIABLE(InstanceColor);
	DEFINE_VARIABLE(InstanceGeometry);

	DEFINE_VARIABLE(BoneToRootMatrices);

	DEFINE_VARIABLE(MainTexture);
	DEFINE_VARIABLE(BumpTexture);
	DEFINE_VARIABLE(SpecularTexture);
	DEFINE_VARIABLE(EmissiveTexture);
	DEFINE_VARIABLE(LightmapTexture);

	DEFINE_VARIABLE(MatrixTextureBuffer);
	DEFINE_VARIABLE(MatrixTextureBufferOffset);

	DEFINE_VARIABLE(SSAOTexture);
	DEFINE_VARIABLE(DepthTexture);
	DEFINE_VARIABLE(ShadowDepthTexture);

	DEFINE_VARIABLE(Gloss);
	DEFINE_VARIABLE(MainColor);
	DEFINE_VARIABLE(SpecularColor);
	DEFINE_VARIABLE(EmissiveColor);

	DEFINE_VARIABLE(DecalMatrix);
	DEFINE_VARIABLE(LocalToClipMatrix);
	DEFINE_VARIABLE(LocalToWorldMatrix);
	DEFINE_VARIABLE(WorldToOrthographicLightMatrix);
};

#undef DEFINE_VARIABLE
