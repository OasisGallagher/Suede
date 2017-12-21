#pragma once
#include <string>
#include <wrappers/gl.h>

#include "texture.h"
#include "internal/file/image.h"
#include "internal/base/objectinternal.h"

class TextureInternal : virtual public ITexture, public ObjectInternal {
public:
	TextureInternal(ObjectType type) :ObjectInternal(type), texture_(0), width_(0), height_(0), location_(0), format_(0) {
	}

public:
	virtual void Bind(uint index);
	virtual void Unbind();
	virtual uint GetNativePointer() { return texture_; }
	
	virtual int GetWidth() { return width_; }
	virtual int GetHeight() { return height_; }

protected:
	virtual GLenum GetGLTextureType() = 0;
	virtual GLenum GetGLTextureBindingName() = 0;

	void BindTexture();
	void UnbindTexture();
	void DestroyTexture();
	BppType GLTextureFormatToBpp(GLenum format);
	void ColorFormatToGLTextureFormat(ColorFormat format, GLenum(&parameters)[3]);

protected:
	int width_, height_;
	GLint oldBindingTexture_;

	GLenum format_;
	GLuint texture_;
	GLenum location_;
};

class Texture2DInternal : public ITexture2D, public TextureInternal {
	DEFINE_FACTORY_METHOD(Texture2D)

public:
	Texture2DInternal();
	~Texture2DInternal();

public:
	virtual bool Load(const std::string& path);
	virtual bool Load(const void* data, ColorFormat format, int width, int height);

	virtual bool EncodeToPng(std::vector<uchar>& data);
	virtual bool EncodeToJpg(std::vector<uchar>& data);

protected:
	virtual GLenum GetGLTextureType() { return GL_TEXTURE_2D; }
	virtual GLenum GetGLTextureBindingName() { return GL_TEXTURE_BINDING_2D; }

private:
	bool EncodeTo(std::vector<uchar>& data, ImageType type);
};

class TextureCubeInternal : public ITextureCube, public TextureInternal {
	DEFINE_FACTORY_METHOD(TextureCube)

public:
	TextureCubeInternal();
	~TextureCubeInternal();

public:
	bool Load(const std::string (&textures)[6]);

protected:
	virtual GLenum GetGLTextureType() { return GL_TEXTURE_CUBE_MAP; }
	virtual GLenum GetGLTextureBindingName() { return GL_TEXTURE_BINDING_CUBE_MAP; }
};

class RenderTextureInternal : public IRenderTexture, public TextureInternal {
	DEFINE_FACTORY_METHOD(RenderTexture)

public:
	RenderTextureInternal();

public:
	bool Load(RenderTextureFormat format, int width, int height);

protected:
	virtual GLenum GetGLTextureType() { return GL_TEXTURE_2D; }
	virtual GLenum GetGLTextureBindingName() { return GL_TEXTURE_BINDING_2D; }
	void RenderTextureFormatToGLenum(RenderTextureFormat input, GLenum (&parameters)[3]);
};
