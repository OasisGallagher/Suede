#pragma once
#include <vector>

#include "object.h"

class SUEDE_API ITexture : virtual public IObject {
public:
	virtual void Bind(uint index) = 0;
	virtual void Unbind() = 0;
	virtual uint GetNativePointer() = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
};

enum ColorFormat {
	ColorFormatRgb,
	ColorFormatBgr,
	ColorFormatRgba,
	ColorFormatArgb,
	ColorFormatBgra,
	ColorFormatLuminanceAlpha,
};

class SUEDE_API ITexture2D : virtual public ITexture {
public:
	virtual bool Load(const std::string& path) = 0;
	virtual bool Load(const void* data, ColorFormat format, int width, int height) = 0;

	virtual bool EncodeToPng(std::vector<uchar>& data) = 0;
	virtual bool EncodeToJpg(std::vector<uchar>& data) = 0;
};

class SUEDE_API ITextureCube : virtual public ITexture {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

enum RenderTextureFormat {
	RenderTextureFormatRgba,
	RenderTextureFormatRgbaHdr,
	RenderTextureFormatDepth,
	RenderTextureFormatShadow,
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
