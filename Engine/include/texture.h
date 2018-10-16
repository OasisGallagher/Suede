#pragma once
#include <vector>

#include "rect.h"
#include "color.h"
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

class SUEDE_API ITexture : public IObject {
	SUEDE_DECLARE_IMPL(Texture)

public:
	void Bind(uint index);
	void Unbind();
	uint GetNativePointer();

	void SetMinFilterMode(TextureMinFilterMode value);
	TextureMinFilterMode GetMinFilterMode() const;

	void SetMagFilterMode(TextureMagFilterMode value);
	TextureMagFilterMode GetMagFilterMode() const;

	void SetWrapModeS(TextureWrapMode value);
	TextureWrapMode GetWrapModeS() const;

	void SetWrapModeT(TextureWrapMode value);
	TextureWrapMode GetWrapModeT() const;

	uint GetWidth() const;
	uint GetHeight() const;

protected:
	ITexture(void* d);
};

enum class TextureFormat {
	Rgb,
	Rgba,
	RgbS,
	RgbaS,
	Rgb16F,
	Rgb32F,
	Rgba16F,
	Rgba32F,
};

enum class ColorStreamFormat {
	Rgb,
	RgbF,
	Bgr,
	Rgba,
	RgbaF,
	Argb,
	Bgra,
	LuminanceAlpha,
};

class SUEDE_API ITexture2D : public ITexture {
	SUEDE_DECLARE_IMPL(Texture2D)

public:
	ITexture2D();

public:
	bool Create(const std::string& path);
	bool Create(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap = false);

	TextureFormat GetFormat();

	bool EncodeToPNG(std::vector<uchar>& data);
	bool EncodeToJPG(std::vector<uchar>& data);
};

class SUEDE_API ITextureCube : public ITexture {
	SUEDE_DECLARE_IMPL(TextureCube)

public:
	ITextureCube();

public:
	bool Load(const std::string textures[6]);
};

class SUEDE_API ITextureBuffer : public ITexture {
	SUEDE_DECLARE_IMPL(TextureBuffer)

public:
	ITextureBuffer();

public:
	uint GetSize() const;
	bool Create(uint size);
	void Update(uint offset, uint size, const void* data);
};

BETTER_ENUM(RenderTextureFormat, int,
	Rgb,
	RgbS,
	RgbF,

	Rgba,
	RgbaS,
	RgbaF,

	Depth,
	Shadow,
	DepthStencil
);

class SUEDE_API IRenderTexture : public ITexture {
	SUEDE_DECLARE_IMPL(RenderTexture)

public:
	IRenderTexture();

public:
	bool Create(RenderTextureFormat format, uint width, uint height);

	void Resize(uint width, uint height);
	void Clear(const Rect& normalizedRect, const Color& color, float depth);

	void BindWrite(const Rect& normalizedRect);

protected:
	IRenderTexture(void* d);
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

class SUEDE_API IMRTRenderTexture : public IRenderTexture {
	SUEDE_DECLARE_IMPL(MRTRenderTexture)

public:
	IMRTRenderTexture();

public:
	bool AddColorTexture(TextureFormat format);
	Texture2D GetColorTexture(uint index);
	uint GetColorTextureCount();
};

SUEDE_DEFINE_OBJECT_POINTER(MRTRenderTexture);
SUEDE_DECLARE_OBJECT_CREATER(MRTRenderTexture);
