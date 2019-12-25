#pragma once
#include <string>

#include "texture.h"
#include "glenums.h"
#include "framebuffer.h"
#include "internal/codec/image.h"
#include "internal/base/objectinternal.h"

struct Sampler {
	TextureWrapMode wrapS = TextureWrapMode::ClampToEdge;
	TextureWrapMode wrapT = TextureWrapMode::ClampToEdge;

	TextureMinFilterMode minFilter = TextureMinFilterMode::Linear;
	TextureMagFilterMode magFilter = TextureMagFilterMode::Linear;
};

class TextureInternal : public ObjectInternal {
public:
	TextureInternal(ObjectType type, Context* context);
	~TextureInternal();

public:
	virtual void Bind(uint index);
	virtual void Unbind();

	virtual uint GetWidth() const { return width_; }
	virtual uint GetHeight() const { return height_; }

public:
	uint GetNativePointer() { return texture_; }

	void SetMinFilterMode(TextureMinFilterMode value);
	TextureMinFilterMode GetMinFilterMode() const { return sampler_.minFilter; }
	
	void SetMagFilterMode(TextureMagFilterMode value);
	TextureMagFilterMode GetMagFilterMode() const { return sampler_.magFilter; }

	void SetWrapModeS(TextureWrapMode value);
	TextureWrapMode GetWrapModeS() const { return sampler_.wrapS; }

	void SetWrapModeT(TextureWrapMode value);
	TextureWrapMode GetWrapModeT() const { return sampler_.wrapT; }

protected:
	void DestroyTexture();

protected:
	virtual uint GetGLTextureType() const = 0;
	virtual uint GetGLTextureBindingName() const = 0;
	virtual bool SupportsSampler() const { return true; }
	virtual void OnContextDestroyed();

	void BindTexture() const;
	void UnbindTexture() const;
	BPPType GLenumToBpp(uint format) const;
	uint TextureFormatToGLenum(TextureFormat textureFormat) const;
	void ColorStreamFormatToGLenum(uint(&parameters)[2], ColorStreamFormat format) const;

private:
	void ApplySampler();

	uint TextureMinFilterModeToGLenum(TextureMinFilterMode mode) const;
	uint TextureMagFilterModeToGLenum(TextureMagFilterMode mode) const;
	uint TextureWrapModeToGLenum(TextureWrapMode mode) const;

	TextureMinFilterMode GLenumToTextureMinFilterMode(uint value) const;
	TextureMagFilterMode GLenumToTextureMagFilterMode(uint value) const;
	TextureWrapMode GLenumToTextureWrapMode(uint value) const;

protected:
	Context* context_;

	Sampler sampler_;
	bool samplerDirty_;

	int width_, height_;
	mutable int oldBindingTexture_;

	uint texture_;
	uint location_;
	uint internalFormat_;
};

class Texture2DInternal : public TextureInternal {
public:
	Texture2DInternal(Context* context);
	~Texture2DInternal();

public:
	bool Load(const std::string& path);
	bool Create(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap = false);

	TextureFormat GetFormat() { return format_; }

	bool EncodeToPNG(std::vector<uchar>& data);
	bool EncodeToJPG(std::vector<uchar>& data);

protected:
	virtual uint GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

private:
	bool EncodeTo(std::vector<uchar>& data, ImageType type);

private:
	TextureFormat format_;
};

class TextureCubeInternal : public TextureInternal {
public:
	TextureCubeInternal(Context* context);
	~TextureCubeInternal();

public:
	bool Load(const std::string textures[6]);

protected:
	virtual uint GetGLTextureType() const { return GL_TEXTURE_CUBE_MAP; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_CUBE_MAP; }
};

class Buffer;
class TextureBufferInternal : public TextureInternal {
public:
	TextureBufferInternal(Context* context);
	~TextureBufferInternal();

public:
	bool Create(uint size);
	uint GetSize() const;
	void Update(uint offset, uint size, const void* data);

protected:
	virtual void OnContextDestroyed();
	virtual uint GetGLTextureType() const { return GL_TEXTURE_BUFFER; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_BUFFER; }
	virtual bool SupportsSampler() const { return false; }

private:
	void DestroyBuffer();

private:
	Buffer* buffer_;
};

class RenderTextureInternal : public TextureInternal {
public:
	RenderTextureInternal(Context* context);
	virtual ~RenderTextureInternal();

public:
	virtual void Bind(uint index);
	virtual void Unbind();

	virtual bool Create(RenderTextureFormat format, uint width, uint height);
	virtual void Resize(uint width, uint height);

	virtual void BindWrite(const Rect& normalizedRect);
	virtual void Clear(const Rect& normalizedRect, const Color& color, float depth);

	RenderTextureFormat GetRenderTextureFormat() { return renderTextureFormat_; }

public:
	static RenderTexture* GetDefault();
	static RenderTexture* GetTemporary(RenderTextureFormat format, uint width, uint height);
	static void ReleaseTemporary(RenderTexture* texture); 

protected:
	virtual uint GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

protected:
	virtual void ResizeStorage(uint w, uint h, RenderTextureFormat format);

protected:
	void DestroyFramebuffer();

protected:
	bool SetViewport(uint width, uint height, const Rect& normalizedRect);

private:
	bool VerifyBindStatus();
	bool ContainsDepthInfo() const { return renderTextureFormat_ >= RenderTextureFormat::Depth; }
	void RenderTextureFormatToGLenum(RenderTextureFormat input, uint(&parameters)[3]);

protected:
	FramebufferBase* framebuffer_;

private:
	enum {
		StatusNone,
		StatusRead,
		StatusWrite,
	} bindStatus_;

	RenderTextureFormat renderTextureFormat_;
};

class ScreenRenderTexture : public RenderTexture {
	SUEDE_DECLARE_IMPLEMENTATION(ScreenRenderTexture);

public:
	ScreenRenderTexture();
};

class ScreenRenderTextureInternal : public RenderTextureInternal {
public:
	ScreenRenderTextureInternal(Context* context);
	virtual ~ScreenRenderTextureInternal();

public:
	virtual uint GetWidth() const;
	virtual uint GetHeight() const;

	virtual void BindWrite(const Rect& normalizedRect);
	virtual void Unbind();

	virtual bool Create(RenderTextureFormat format, uint width, uint height);
	virtual void Clear(const Rect& normalizedRect, const Color& color, float depth);

protected:
	virtual void Resize(uint width, uint height);
	virtual uint GetGLTextureType() const;
	virtual uint GetGLTextureBindingName() const;
};

class MRTRenderTextureInternal : public RenderTextureInternal {
public:
	MRTRenderTextureInternal(Context* context) : RenderTextureInternal(context), index_(0) {}

public:
	virtual bool Create(RenderTextureFormat format, uint width, uint height);
	virtual void Resize(uint width, uint height);

	virtual void Bind(uint index);
	virtual void BindWrite(const Rect& normalizedRect);

public:
	virtual bool AddColorTexture(TextureFormat format);
	virtual uint GetColorTextureCount() { return index_; }
	virtual Texture2D* GetColorTexture(uint index);

protected:
	virtual uint GetGLTextureType() const;
	virtual uint GetGLTextureBindingName() const;

private:
	void DestroyColorTextures();

private:
	uint index_;
	ref_ptr<Texture2D> colorTextures_[FramebufferAttachmentMax];
};
