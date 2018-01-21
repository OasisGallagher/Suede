#pragma once
#include <string>
#include <wrappers/gl.h>

#include "texture.h"
#include "internal/file/image.h"
#include "internal/base/objectinternal.h"

class TextureInternal : virtual public ITexture, public ObjectInternal {
public:
	TextureInternal(ObjectType type);

public:
	virtual void Bind(uint index);
	virtual void Unbind();
	virtual uint GetNativePointer() { return texture_; }
	
	virtual int GetWidth() { return width_; }
	virtual int GetHeight() { return height_; }

	virtual void SetMinFilterMode(TextureMinFilterMode value);
	virtual TextureMinFilterMode GetMinFilterMode() const;

	virtual void SetMagFilterMode(TextureMagFilterMode value);
	virtual TextureMagFilterMode GetMagFilterMode() const;

	virtual void SetWrapModeS(TextureWrapMode value);
	virtual TextureWrapMode GetWrapModeS() const;

	virtual void SetWrapModeT(TextureWrapMode value);
	virtual TextureWrapMode GetWrapModeT() const;

protected:
	virtual GLenum GetGLTextureType() const = 0;
	virtual GLenum GetGLTextureBindingName() const = 0;

	void BindTexture() const;
	void UnbindTexture() const;
	void DestroyTexture();
	BppType GLenumToBpp(GLenum format) const;
	GLenum TextureFormatToGLenum(TextureFormat textureFormat) const;
	void ColorStreamFormatToGLenum(GLenum(&parameters)[2], ColorStreamFormat format) const;

private:
	GLenum TextureMinFilterModeToGLenum(TextureMinFilterMode mode) const;
	GLenum TextureMagFilterModeToGLenum(TextureMagFilterMode mode) const;
	GLenum TextureWrapModeToGLenum(TextureWrapMode mode) const;

	TextureMinFilterMode GLenumToTextureMinFilterMode(GLenum value) const;
	TextureMagFilterMode GLenumToTextureMagFilterMode(GLenum value) const;
	TextureWrapMode GLenumToTextureWrapMode(GLenum value) const;

protected:
	int width_, height_;
	mutable GLint oldBindingTexture_;

	GLuint texture_;
	GLenum location_;
	GLenum internalFormat_;
};

class Texture2DInternal : public ITexture2D, public TextureInternal {
	DEFINE_FACTORY_METHOD(Texture2D)

public:
	Texture2DInternal();
	~Texture2DInternal();

public:
	virtual bool Load(const std::string& path);
	virtual bool Load(TextureFormat textureFormat, const void* data, ColorStreamFormat format, int width, int height, bool mipmap = false);

	virtual bool EncodeToPng(std::vector<uchar>& data);
	virtual bool EncodeToJpg(std::vector<uchar>& data);

protected:
	virtual GLenum GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual GLenum GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

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
	virtual GLenum GetGLTextureType() const { return GL_TEXTURE_CUBE_MAP; }
	virtual GLenum GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_CUBE_MAP; }
};

class RenderTextureInternal : public IRenderTexture, public TextureInternal {
	DEFINE_FACTORY_METHOD(RenderTexture)

public:
	RenderTextureInternal();

public:
	bool Load(RenderTextureFormat format, int width, int height);

protected:
	virtual GLenum GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual GLenum GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }
	void RenderTextureFormatToGLenum(RenderTextureFormat input, GLenum (&parameters)[3]);
};
