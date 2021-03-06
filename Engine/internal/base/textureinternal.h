#pragma once
#include <string>

#include "texture.h"
#include "glenums.h"
#include "framebuffer.h"
#include "globjectmaintainer.h"
#include "internal/codec/image.h"
#include "internal/base/objectinternal.h"

struct Sampler {
	TextureWrapMode wrapS = TextureWrapMode::ClampToEdge;
	TextureWrapMode wrapT = TextureWrapMode::ClampToEdge;

	TextureMinFilterMode minFilter = TextureMinFilterMode::Linear;
	TextureMagFilterMode magFilter = TextureMagFilterMode::Linear;
};

class TextureInternal : public ObjectInternal, public GLObjectMaintainer {
public:
	TextureInternal(ObjectType type, Context* context);
	~TextureInternal();

public:
	virtual void Bind(uint index);
	virtual void Unbind();

	virtual uint GetWidth() const { return width_; }
	virtual uint GetHeight() const { return height_; }

public:
	uint GetNativePointer();

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
	uint TextureFormatToGLenum(TextureFormat textureFormat) const;
	void ColorStreamFormatToGLenum(uint& glFormat, uint& glType, ColorStreamFormat format) const;

private:
	void ApplySampler();

	uint TextureMinFilterModeToGLenum(TextureMinFilterMode mode) const;
	uint TextureMagFilterModeToGLenum(TextureMagFilterMode mode) const;
	uint TextureWrapModeToGLenum(TextureWrapMode mode) const;

	TextureMinFilterMode GLenumToTextureMinFilterMode(uint value) const;
	TextureMagFilterMode GLenumToTextureMagFilterMode(uint value) const;
	TextureWrapMode GLenumToTextureWrapMode(uint value) const;

protected:
	Sampler sampler_;
	bool samplerDirty_ = true;

	int width_ = 0, height_ = 0;
	mutable int oldBindingTexture_ = 0;

	bool mipmap_ = false;
	uint texture_ = 0;
	uint location_ = 0;

	uint glType_ = 0, glFormat_ = 0;
	uint internalFormat_ = 0;
};

class Texture2DInternal : public TextureInternal {
public:
	Texture2DInternal(Context* context);
	~Texture2DInternal();

public:
	bool Load(const std::string& path);
	bool Create(TextureFormat textureFormat, const void* data, ColorStreamFormat colorStreamFormat, uint width, uint height, uint alignment, bool mipmap = false);

	TextureFormat GetFormat() { return textureFormat_; }

	bool EncodeToPNG(std::vector<uchar>& data);
	bool EncodeToJPG(std::vector<uchar>& data);

protected:
	virtual void Bind(uint index);
	virtual uint GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

private:
	void ApplyData();
	bool EncodeTo(std::vector<uchar>& data, ImageType type);

private:
	bool dataDirty_ = false;

	uint shadowDataSize_;
	std::unique_ptr<uchar[]> shadowData_;

	int alignment_ = 4;
	TextureFormat textureFormat_ = TextureFormat::Rgba;
	ColorStreamFormat colorStreamFormat_ = ColorStreamFormat::Rgba;
};

class TextureCubeInternal : public TextureInternal {
public:
	TextureCubeInternal(Context* context);
	~TextureCubeInternal();

public:
	bool Load(const std::string textures[6]);

protected:
	virtual void Bind(uint index);
	virtual uint GetGLTextureType() const { return GL_TEXTURE_CUBE_MAP; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_CUBE_MAP; }

private:
	void ApplyContent();

private:
	std::unique_ptr<RawImage[]> rawImages_;
};

class Buffer;
class TextureBufferInternal : public TextureInternal {
public:
	TextureBufferInternal(Context* context);
	~TextureBufferInternal();

public:
	bool Create(uint size);

	uint GetSize() const { return size_; }
	void Update(uint offset, uint size, const void* data);

protected:
	virtual void Bind(uint index);
	virtual void OnContextDestroyed();
	virtual uint GetGLTextureType() const { return GL_TEXTURE_BUFFER; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_BUFFER; }
	virtual bool SupportsSampler() const { return false; }

private:
	void ApplySize();
	void ApplyContent();
	void DestroyBuffer();

private:
	uint size_ = 0;
	bool sizeDirty_ = false;

	struct {
		uint offset, size;
		std::unique_ptr<uchar[]> data;
	} newContentArgument_;

	Buffer* buffer_ = nullptr;
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
	virtual void Clear(const Rect& normalizedRect, const Color& color, float depth, int stencil);

	void __tmpApplyClear() { if (clearArgument_.dirty) { ApplyClearContent(); } }

	RenderTextureFormat GetRenderTextureFormat() { return renderTextureFormat_; }

protected:
	virtual void OnContextDestroyed();
	virtual uint GetGLTextureType() const { return GL_TEXTURE_2D; }
	virtual uint GetGLTextureBindingName() const { return GL_TEXTURE_BINDING_2D; }

protected:
	virtual void ApplySize();
	virtual void ApplyConfig();
	virtual void ResizeStorage(uint w, uint h, RenderTextureFormat renderTextureFormat);

protected:
	void DestroyFramebuffer();
	void ApplyClearContent();

protected:
	bool SetViewport(const Rect& viewport);

private:
	bool VerifyBindStatus();
	bool IsDepthOrStencilTexture() const { return renderTextureFormat_ >= RenderTextureFormat::Depth; }
	void RenderTextureFormatToGLenum(RenderTextureFormat input, uint& internalFormat, uint& format, uint& type);

protected:
	FramebufferBase* framebuffer_ = nullptr;

	enum {
		StatusNone,
		StatusRead,
		StatusWrite,
	} bindStatus_ = StatusNone;

	bool sizeDirty_ = false;
	bool configDirty_ = false;

	struct {
		bool dirty = false;

		Rect normalizedRect;
		Color color;
		float depth = -1;
		int stencil = -1;
	} clearArgument_;

	RenderTextureFormat renderTextureFormat_ = RenderTextureFormat::Rgba;
};

class ScreenRenderTexture : public RenderTexture {
	SUEDE_DECLARE_IMPLEMENTATION(ScreenRenderTexture)

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

	virtual void Bind(uint index);
	virtual void BindWrite(const Rect& normalizedRect);
	virtual void Unbind();

	virtual bool Create(RenderTextureFormat format, uint width, uint height);

protected:
	virtual void OnContextDestroyed();
	virtual void Resize(uint width, uint height);
	virtual uint GetGLTextureType() const;
	virtual uint GetGLTextureBindingName() const;
};

class MRTRenderTextureInternal : public RenderTextureInternal {
public:
	MRTRenderTextureInternal(Context* context) : RenderTextureInternal(context) {}

public:
	virtual bool Create(RenderTextureFormat format, uint width, uint height);
	virtual void Resize(uint width, uint height);

	virtual void Bind(uint index);
	virtual void BindWrite(const Rect& normalizedRect);

public:
	virtual bool AddColorTexture(TextureFormat format);
	virtual uint GetColorTextureCount() { return currentIndex_; }
	virtual Texture2D* GetColorTexture(uint index);

protected:
	virtual void ApplySize();
	virtual void ApplyConfig();
	virtual uint GetGLTextureType() const;
	virtual uint GetGLTextureBindingName() const;

private:
	void DestroyColorTextures();

private:
	uint currentIndex_ = 0;
	uint newColorTextureFrom_ = 0;
	ref_ptr<Texture2D> colorTextures_[FramebufferAttachmentMax];
};
