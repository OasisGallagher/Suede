#pragma once
#include <vector>

#include "rect.h"
#include "object.h"
#include "tools/enum.h"

BETTER_ENUM(TextureMinFilterMode, int,
	Nearest,
	Linear,
	NearestMipmapNearest,
	LinearMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapLinear
)

BETTER_ENUM(TextureMagFilterMode, int,
	Nearest,
	Linear
)

BETTER_ENUM(TextureWrapMode, int,
	ClampToEdge,
	MirroredRepeat,
	Repeat
)

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

	virtual uint GetWidth() const = 0;
	virtual uint GetHeight() const = 0;
};

enum TextureFormat {
	TextureFormatRgb,
	TextureFormatRgba,
	TextureFormatRgbS,
	TextureFormatRgbaS,
	TextureFormatRgb16F,
	TextureFormatRgb32F,
	TextureFormatRgba16F,
	TextureFormatRgba32F,
};

enum ColorStreamFormat {
	ColorStreamFormatRgb,
	ColorStreamFormatRgbF,
	ColorStreamFormatBgr,
	ColorStreamFormatRgba,
	ColorStreamFormatRgbaF,
	ColorStreamFormatArgb,
	ColorStreamFormatBgra,
	ColorStreamFormatLuminanceAlpha,
};

class SUEDE_API ITexture2D : virtual public ITexture {
public:
	virtual bool Create(const std::string& path) = 0;
	virtual bool Create(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap = false) = 0;

	virtual TextureFormat GetFormat() = 0;

	virtual bool EncodeToPNG(std::vector<uchar>& data) = 0;
	virtual bool EncodeToJPG(std::vector<uchar>& data) = 0;
};

class SUEDE_API ITextureCube : virtual public ITexture {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

class SUEDE_API ITextureBuffer : virtual public ITexture {
public:
	virtual uint GetSize() const = 0;
	virtual bool Create(uint size) = 0;
	virtual void Update(uint offset, uint size, const void* data) = 0;
};

enum RenderTextureFormat {
	RenderTextureFormatRgb,
	RenderTextureFormatRgbSN,
	RenderTextureFormatRgbHDR,

	RenderTextureFormatRgba,
	RenderTextureFormatRgbaSN,
	RenderTextureFormatRgbaHDR,

	RenderTextureFormatDepth,
	RenderTextureFormatShadow,
	RenderTextureFormatDepthStencil,
};

class SUEDE_API IRenderTexture : virtual public ITexture {
public:
	virtual bool Create(RenderTextureFormat format, uint width, uint height) = 0;

	virtual void Resize(uint width, uint height) = 0;
	virtual void Clear(const Rect& normalizedRect, const glm::vec4& value) = 0;

	virtual void BindWrite(const Rect& normalizedRect) = 0;
};

SUEDE_DEFINE_CUSTOM_OBJECT_POINTER(RenderTexture) {
	SUEDE_IMPLEMENT_CUSTOM_OBJECT_POINTER(RenderTexture)

	static RenderTexture GetDefault();

	static RenderTexture GetTemporary(RenderTextureFormat format, uint width, uint height);
	static void ReleaseTemporary(RenderTexture texture);
};

SUEDE_DEFINE_OBJECT_POINTER(Texture);
SUEDE_DEFINE_OBJECT_POINTER(Texture2D);
SUEDE_DEFINE_OBJECT_POINTER(TextureCube);
SUEDE_DEFINE_OBJECT_POINTER(TextureBuffer);

SUEDE_DECLARE_OBJECT_CREATER(Texture2D);
SUEDE_DECLARE_OBJECT_CREATER(TextureCube);
SUEDE_DECLARE_OBJECT_CREATER(RenderTexture);
SUEDE_DECLARE_OBJECT_CREATER(TextureBuffer);

class SUEDE_API IMRTRenderTexture : virtual public IRenderTexture {
public:
	virtual bool AddColorTexture(TextureFormat format) = 0;
	virtual Texture2D GetColorTexture(uint index) = 0;
	virtual uint GetColorTextureCount() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(MRTRenderTexture);
SUEDE_DECLARE_OBJECT_CREATER(MRTRenderTexture);
