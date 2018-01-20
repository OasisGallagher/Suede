#pragma once
#include <vector>

#include "object.h"

enum TextureMinFilterMode {
	TextureMinFilterModeNearest,
	TextureMinFilterModeLinear,
	TextureMinFilterModeNearestMipmapNearest,
	TextureMinFilterModeLinearMipmapNearest,
	TextureMinFilterModeNearestMipmapLinear,
	TextureMinFilterModeLinearMipmapLinear,
};

enum TextureMagFilterMode {
	TextureMagFilterModeNearest,
	TextureMagFilterModeLinear,
};

enum TextureWrapMode {
	TextureWrapModeClampToEdge,
	TextureWrapModeMirroredRepeat,
	TextureWrapModeRepeat,
};

class SUEDE_API ITexture : virtual public IObject {
public:
	virtual void Bind(uint index) = 0;
	virtual void Unbind() = 0;
	virtual uint GetNativePointer() = 0;

	virtual void SetMinFilterMode(TextureMinFilterMode value) = 0;
	virtual TextureMinFilterMode GetMinFilterMode() const = 0;

	virtual void SetMagFilterMode(TextureMagFilterMode value) = 0;
	virtual TextureMagFilterMode GetMagFilterMode() const = 0;

	virtual void SetWrapModeS(TextureWrapMode value) = 0;
	virtual TextureWrapMode GetWrapModeS() const = 0;

	virtual void SetWrapModeT(TextureWrapMode value) = 0;
	virtual TextureWrapMode GetWrapModeT() const = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
};

enum TextureFormat {
	TextureFormatRgb,
	TextureFormatRgba,
	TextureFormatRgbS,
	TextureFormatRgbaS,
	TextureFormatRgb16,
	TextureFormatRgb32,
	TextureFormatRgba16,
	TextureFormatRgba32,
};

enum ColorStreamFormat {
	ColorStreamFormatRgb,
	ColorStreamFormatBgr,
	ColorStreamFormatRgba,
	ColorStreamFormatArgb,
	ColorStreamFormatBgra,
	ColorStreamFormatLuminanceAlpha,
};

class SUEDE_API ITexture2D : virtual public ITexture {
public:
	virtual bool Load(const std::string& path) = 0;
	virtual bool Load(TextureFormat textureFormat, const void* data, ColorStreamFormat format, int width, int height) = 0;

	virtual bool EncodeToPng(std::vector<uchar>& data) = 0;
	virtual bool EncodeToJpg(std::vector<uchar>& data) = 0;
};

class SUEDE_API ITextureCube : virtual public ITexture {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

enum RenderTextureFormat {
	RenderTextureFormatRgba,
	RenderTextureFormatRgbaSn,
	RenderTextureFormatRgbHdr,
	RenderTextureFormatRgbaHdr,
	RenderTextureFormatDepth,
	RenderTextureFormatShadow,
	RenderTextureFormatDepthStencil,
};

class SUEDE_API IRenderTexture : virtual public ITexture {
public:
	virtual bool Load(RenderTextureFormat format, int width, int height) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Texture);
SUEDE_DEFINE_OBJECT_POINTER(Texture2D);
SUEDE_DEFINE_OBJECT_POINTER(TextureCube);
SUEDE_DEFINE_OBJECT_POINTER(RenderTexture);

SUEDE_DECLARE_OBJECT_CREATER(Texture2D);
SUEDE_DECLARE_OBJECT_CREATER(TextureCube);
SUEDE_DECLARE_OBJECT_CREATER(RenderTexture);
