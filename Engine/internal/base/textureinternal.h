#pragma once
#include <string>

#include "texture.h"
#include "../api/gl.h"
#include "framebuffer.h"
#include "internal/codec/image.h"
#include "internal/base/objectinternal.h"

class TextureInternal : virtual public ITexture, public ObjectInternal {
public:
	TextureInternal(ObjectType type);
	~TextureInternal();

public:
	virtual void Bind(uint index);
	virtual void Unbind();
	virtual uint GetNativePointer() { return texture_; }
	
	virtual uint GetWidth() const { return width_; }
	virtual uint GetHeight() const { return height_; }

	virtual void SetMinFilterMode(TextureMinFilterMode value);
	virtual TextureMinFilterMode GetMinFilterMode() const;

	virtual void SetMagFilterMode(TextureMagFilterMode value);
	virtual TextureMagFilterMode GetMagFilterMode() const;

	virtual void SetWrapModeS(TextureWrapMode value);
	virtual TextureWrapMode GetWrapModeS() const;

	virtual void SetWrapModeT(TextureWrapMode value);
	virtual TextureWrapMode GetWrapModeT() const;

protected:
	void DestroyTexture();

protected:
	virtual GLenum GetGLTextureType() const = 0;
	virtual GLenum GetGLTextureBindingName() const = 0;

	void BindTexture() const;
	void UnbindTexture() const;
	BPPType GLenumToBpp(GLenum format) const;
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
	virtual bool Create(const std::string& path);
	virtual bool Create(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap = false);

	virtual TextureFormat GetFormat() { return format_; }

	virtual bool EncodeToPNG(std::vector<uchar>& data);
	virtual bool EncodeToJPG(std::vector<uchar>& data);

protected:
	virtual GLenum GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual GLenum GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

private:
	bool EncodeTo(std::vector<uchar>& data, ImageType type);

private:
	TextureFormat format_;
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

class Buffer;
class TextureBufferInternal : public ITextureBuffer, public TextureInternal {
	DEFINE_FACTORY_METHOD(TextureBuffer)

public:
	TextureBufferInternal();
	~TextureBufferInternal();

public:
	bool Create(uint size);
	uint GetSize() const;
	void Update(uint offset, uint size, const void* data);

protected:
	virtual GLenum GetGLTextureType() const { return GL_TEXTURE_BUFFER; }
	virtual GLenum GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_BUFFER; }

private:
	void DestroyBuffer();

private:
	Buffer* buffer_;
};

class RenderTextureInternalBase : virtual public IRenderTexture, public TextureInternal {
public:
	RenderTextureInternalBase() : TextureInternal(ObjectTypeRenderTexture), framebuffer_(nullptr) {}

protected:
	bool SetViewport(uint width, uint height, const Rect& normalizedRect);

protected:
	FramebufferBase* framebuffer_;
};

class RenderTextureInternal : public RenderTextureInternalBase {
	DEFINE_FACTORY_METHOD(RenderTexture)

public:
	RenderTextureInternal();
	~RenderTextureInternal();

public:
	virtual bool Create(RenderTextureFormat format, uint width, uint height);

	virtual void Resize(uint width, uint height);
	virtual void Clear(const Rect& normalizedRect, const glm::vec4& value);

	virtual void BindWrite(const Rect& normalizedRect);

	virtual void Bind(uint index);
	virtual void Unbind();

protected:
	virtual GLenum GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual GLenum GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

protected:
	virtual void ResizeStorage(uint w, uint h, RenderTextureFormat format);

protected:
	void DestroyFramebuffer();

private:
	bool VerifyBindStatus();
	bool ContainsDepthInfo() const { return format_ >= RenderTextureFormatDepth; }
	void RenderTextureFormatToGLenum(RenderTextureFormat input, GLenum(&parameters)[3]);

private:
	enum {
		StatusNone,
		StatusRead,
		StatusWrite,
	} bindStatus_;

	RenderTextureFormat format_;
};

class ScreenRenderTextureInternal : public RenderTextureInternalBase {
public:
	ScreenRenderTextureInternal();

protected:
	virtual bool Create(RenderTextureFormat format, uint width, uint height);
	virtual void Clear(const Rect& normalizedRect, const glm::vec4& value);

	virtual uint GetWidth() const;
	virtual uint GetHeight() const;

	virtual void BindWrite(const Rect& normalizedRect);
	virtual void Unbind();

protected:
	virtual void Resize(uint width, uint height);
	virtual GLenum GetGLTextureType() const;
	virtual GLenum GetGLTextureBindingName() const;
};

class MRTRenderTextureInternal : public RenderTextureInternal, public IMRTRenderTexture {
	DEFINE_FACTORY_METHOD(MRTRenderTexture)

public:
	MRTRenderTextureInternal() : index_(0) {}

public:
	virtual bool Create(RenderTextureFormat format, uint width, uint height);
	virtual void Resize(uint width, uint height);

	virtual void Bind(uint index);
	virtual void BindWrite(const Rect& normalizedRect);

public:
	virtual bool AddColorTexture(TextureFormat format);
	virtual uint GetColorTextureCount() { return index_; }
	virtual Texture2D GetColorTexture(uint index);

protected:
	virtual GLenum GetGLTextureType() const;
	virtual GLenum GetGLTextureBindingName() const;

private:
	void DestroyColorTextures();

private:
	uint index_;
	Texture2D colorTextures_[FramebufferAttachmentMax];
};
