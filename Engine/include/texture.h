#pragma once
#include <vector>

#include "object.h"

class ENGINE_EXPORT ITexture : virtual public IObject {
public:
	virtual void Bind(unsigned index) = 0;
	virtual void Unbind() = 0;
	virtual unsigned GetNativePointer() = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
};

enum ColorFormat {
	ColorFormatRgb,
	ColorFormatRgba,
	ColorFormatIntensity,
	ColorFormatLuminanceAlpha,
};

class ENGINE_EXPORT ITexture2D : virtual public ITexture {
public:
	virtual bool Load(const std::string& path) = 0;
	virtual bool Load(const void* data, ColorFormat format, int width, int height) = 0;

	virtual bool EncodeToPng(std::vector<unsigned char>& data) = 0;
	virtual bool EncodeToJpg(std::vector<unsigned char>& data) = 0;
};

class ENGINE_EXPORT ITextureCube : virtual public ITexture {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

enum RenderTextureFormat {
	RenderTextureFormatRgba,
	RenderTextureFormatRgbaHdr,
	RenderTextureFormatDepth,
	RenderTextureFormatShadow,
};

class ENGINE_EXPORT IRenderTexture : virtual public ITexture {
public:
	virtual bool Load(RenderTextureFormat format, int width, int height) = 0;
};

DEFINE_OBJECT_PTR(Texture);
DEFINE_OBJECT_PTR(Texture2D);
DEFINE_OBJECT_PTR(TextureCube);
DEFINE_OBJECT_PTR(RenderTexture);
