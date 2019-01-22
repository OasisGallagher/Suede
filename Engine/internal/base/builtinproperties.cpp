#include "builtinproperties.h"

#include <vector>

#define BUILTIN_PROPERTY_PREFIX					"_"
#define BUILTIN_PROPERTY_NAME(name)				(BUILTIN_PROPERTY_PREFIX #name)
#define DEFINE_BUILTIN_PROPERTY(name)			const char* name = BUILTIN_PROPERTY_NAME(name);
#define DEFINE_BUILTIN_COLOR_PROPERTY(name)		const char* name = DefineBuiltinColorProperty(BUILTIN_PROPERTY_NAME(name))

static std::vector<std::string> colorProperties;
static const char* DefineBuiltinColorProperty(const char* name) {
	colorProperties.push_back(name);
	return name;
}

namespace BuiltinProperties {
	DEFINE_BUILTIN_PROPERTY(BoneToRootMatrices);

	DEFINE_BUILTIN_PROPERTY(MainTexture);
	DEFINE_BUILTIN_PROPERTY(BumpTexture);
	DEFINE_BUILTIN_PROPERTY(SpecularTexture);
	DEFINE_BUILTIN_PROPERTY(EmissiveTexture);
	DEFINE_BUILTIN_PROPERTY(LightmapTexture);

	DEFINE_BUILTIN_PROPERTY(MatrixTextureBuffer);
	DEFINE_BUILTIN_PROPERTY(MatrixTextureBufferOffset);

	DEFINE_BUILTIN_PROPERTY(SSAOTexture);
	DEFINE_BUILTIN_PROPERTY(DepthTexture);
	DEFINE_BUILTIN_PROPERTY(ShadowDepthTexture);

	DEFINE_BUILTIN_PROPERTY(Gloss);

	DEFINE_BUILTIN_COLOR_PROPERTY(MainColor);
	DEFINE_BUILTIN_COLOR_PROPERTY(SpecularColor);
	DEFINE_BUILTIN_COLOR_PROPERTY(EmissiveColor);

	DEFINE_BUILTIN_PROPERTY(DecalMatrix);
	DEFINE_BUILTIN_PROPERTY(LocalToClipMatrix);
	DEFINE_BUILTIN_PROPERTY(LocalToWorldMatrix);
	DEFINE_BUILTIN_PROPERTY(WorldToOrthographicLightMatrix);

	bool IsBuiltinColorProperty(const char* name) {
		return std::find(colorProperties.begin(), colorProperties.end(), name) != colorProperties.end();
	}
}
