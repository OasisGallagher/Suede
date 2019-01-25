#pragma once
#include "types.h"

#define DECLARE_BUILTIN_PROPERTY(name)			SUEDE_API extern const char* name;

namespace BuiltinProperties {
	bool IsBuiltinColorProperty(const char* name);

	DECLARE_BUILTIN_PROPERTY(BoneToRootMatrices);

	DECLARE_BUILTIN_PROPERTY(MainTexture);
	DECLARE_BUILTIN_PROPERTY(BumpTexture);
	DECLARE_BUILTIN_PROPERTY(SpecularTexture);
	DECLARE_BUILTIN_PROPERTY(EmissiveTexture);
	DECLARE_BUILTIN_PROPERTY(LightmapTexture);

	DECLARE_BUILTIN_PROPERTY(SSAOTexture);
	DECLARE_BUILTIN_PROPERTY(DepthTexture);
	DECLARE_BUILTIN_PROPERTY(ShadowDepthTexture);

	DECLARE_BUILTIN_PROPERTY(MatrixTextureBuffer);
	DECLARE_BUILTIN_PROPERTY(MatrixTextureBufferOffset);

	DECLARE_BUILTIN_PROPERTY(DecalMatrix);
	DECLARE_BUILTIN_PROPERTY(LocalToClipMatrix);
	DECLARE_BUILTIN_PROPERTY(LocalToWorldMatrix);
	DECLARE_BUILTIN_PROPERTY(WorldToOrthographicLightMatrix);

	DECLARE_BUILTIN_PROPERTY(Gloss);

	// color properties.
	DECLARE_BUILTIN_PROPERTY(MainColor);
	DECLARE_BUILTIN_PROPERTY(SpecularColor);
	DECLARE_BUILTIN_PROPERTY(EmissiveColor);
};

#undef DECLARE_BUILTIN_PROPERTY
