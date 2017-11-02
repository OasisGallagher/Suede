#pragma once
#include <vector>
#include <gl/glew.h>

#include "object.h"

class ENGINE_EXPORT ITexture : virtual public IObject {
public:
	virtual void Bind(GLenum location) = 0;
	virtual void Unbind() = 0;
	virtual unsigned GetNativePointer() = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
};

class ENGINE_EXPORT ITexture2D : virtual public ITexture {
public:
	virtual bool Load(const std::string& path) = 0;
	virtual bool Load(const void* data, int width, int height) = 0;

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

typedef std::shared_ptr<ITexture> Texture;
typedef std::shared_ptr<ITexture2D> Texture2D;
typedef std::shared_ptr<ITextureCube> TextureCube;
typedef std::shared_ptr<IRenderTexture> RenderTexture;
