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
	SUEDE_DEFINE_METATABLE_NAME(Texture)
	SUEDE_DECLARE_IMPLEMENTATION(Texture)

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

SUEDE_DEFINE_OBJECT_POINTER(Texture)

enum class TextureFormat {
	Rgb,
	Rgba,
	RgbS,
	RgbaS,
	Rgb16F,
	Rgb32F,
	Rgba16F,
	Rgba32F,

	Luminance,
};

enum class ColorStreamFormat {
	Rgb,
	RgbF,
	Bgr,
	Rgba,
	RgbaF,
	Argb,
	Bgra,
	Luminance,
	LuminanceAlpha,
};

class SUEDE_API ITexture2D : public ITexture {
	SUEDE_DEFINE_METATABLE_NAME(Texture2D)
	SUEDE_DECLARE_IMPLEMENTATION(Texture2D)

public:
	ITexture2D();

public:
	bool Load(const std::string& path);
	bool SetPixels(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap = false);

	TextureFormat GetFormat();

	bool EncodeToPNG(std::vector<uchar>& data);
	bool EncodeToJPG(std::vector<uchar>& data);
};

SUEDE_DEFINE_OBJECT_POINTER(Texture2D)

class SUEDE_API ITextureCube : public ITexture {
	SUEDE_DEFINE_METATABLE_NAME(TextureCube)
	SUEDE_DECLARE_IMPLEMENTATION(TextureCube)

public:
	ITextureCube();

public:
	bool Load(const std::string textures[6]);
};

SUEDE_DEFINE_OBJECT_POINTER(TextureCube)

class SUEDE_API ITextureBuffer : public ITexture {
	SUEDE_DEFINE_METATABLE_NAME(TextureBuffer)
	SUEDE_DECLARE_IMPLEMENTATION(TextureBuffer)

public:
	ITextureBuffer();

public:
	uint GetSize() const;
	bool Create(uint size);
	void Update(uint offset, uint size, const void* data);
};

SUEDE_DEFINE_OBJECT_POINTER(TextureBuffer)

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
)

class SUEDE_API IRenderTexture : public ITexture {
	SUEDE_DEFINE_METATABLE_NAME(RenderTexture)
	SUEDE_DECLARE_IMPLEMENTATION(RenderTexture)

public:
	IRenderTexture();

public:
	bool Create(RenderTextureFormat format, uint width, uint height);

	Texture2D ToTexture2D();

	void Resize(uint width, uint height);
	void Clear(const Rect& normalizedRect, const Color& color, float depth);
	void Clear(const Rect& normalizedRect, const Color& color, float depth, int stencil);

	void BindWrite(const Rect& normalizedRect);

protected:
	IRenderTexture(void* d);
};

SUEDE_DEFINE_OBJECT_POINTER(RenderTexture)

struct SUEDE_API RenderTextureUtility {
	static RenderTexture GetDefault();

	static RenderTexture GetTemporary(RenderTextureFormat format, uint width, uint height);
	static void ReleaseTemporary(RenderTexture texture);
};

class SUEDE_API IMRTRenderTexture : public IRenderTexture {
	SUEDE_DEFINE_METATABLE_NAME(MRTRenderTexture)
	SUEDE_DECLARE_IMPLEMENTATION(MRTRenderTexture)

public:
	IMRTRenderTexture();

public:
	bool AddColorTexture(TextureFormat format);
	Texture2D GetColorTexture(uint index);
	uint GetColorTextureCount();
};

SUEDE_DEFINE_OBJECT_POINTER(MRTRenderTexture)
