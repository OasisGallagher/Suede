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

class SUEDE_API Texture : public Object {
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
	Texture(void* d);
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

static int ColorStreamChannels(ColorStreamFormat format) {
	switch (format) {
	case ColorStreamFormat::Rgb: return 3;
	case ColorStreamFormat::RgbF: return 3;
	case ColorStreamFormat::Bgr: return 3;
	case ColorStreamFormat::Rgba: return 4;
	case ColorStreamFormat::RgbaF: return 4;
	case ColorStreamFormat::Argb: return 4;
	case ColorStreamFormat::Bgra: return 4;
	case ColorStreamFormat::LuminanceAlpha: return 2;
	}

	return 0;
}

static int ColorStreamBytes(ColorStreamFormat format) {
	switch (format) {
	case ColorStreamFormat::RgbF: return 3 * sizeof(float);
	case ColorStreamFormat::RgbaF: return 4 * sizeof(float);
	}

	return ColorStreamChannels(format);
}

class SUEDE_API Texture2D : public Texture {
	SUEDE_DEFINE_METATABLE_NAME(Texture2D)
	SUEDE_DECLARE_IMPLEMENTATION(Texture2D)

public:
	Texture2D();

public:
	static Texture2D* GetBlackTexture();
	static Texture2D* GetWhiteTexture();

public:
	bool Load(const std::string& path);
	bool Create(TextureFormat textureFormat, const void* data, ColorStreamFormat colorStreamFormat, uint width, uint height, uint alignment, bool mipmap = false);

	TextureFormat GetFormat();

	bool EncodeToPNG(std::vector<uchar>& data);
	bool EncodeToJPG(std::vector<uchar>& data);
};

class SUEDE_API TextureCube : public Texture {
	SUEDE_DEFINE_METATABLE_NAME(TextureCube)
	SUEDE_DECLARE_IMPLEMENTATION(TextureCube)

public:
	TextureCube();

public:
	bool Load(const std::string textures[6]);
};

class SUEDE_API TextureBuffer : public Texture {
	SUEDE_DEFINE_METATABLE_NAME(TextureBuffer)
	SUEDE_DECLARE_IMPLEMENTATION(TextureBuffer)

public:
	TextureBuffer();

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
)

class SUEDE_API RenderTexture : public Texture {
	SUEDE_DEFINE_METATABLE_NAME(RenderTexture)
	SUEDE_DECLARE_IMPLEMENTATION(RenderTexture)

public:
	RenderTexture();
	~RenderTexture() {}

public:
	void __tmpApplyClear();
	bool Create(RenderTextureFormat format, uint width, uint height);

	RenderTextureFormat GetRenderTextureFormat();

	void Resize(uint width, uint height);

	/**
	 * Clear RenderTexture.
	 * @param normalizedRect Rectangle area to clear.
	 * @param color Value used to clear color buffer.
	 * @param depth Value used to clear depth buffer, clamped to the range [0, 1]. If value equals to -1, depth buffer will not be cleared.
	 * @param stencil Value used to clear stencil buffer, clamped to the range [0, 255]. If value equals to -1, stencil buffer will not be cleared.
	 */
	void Clear(const Rect& normalizedRect, const Color& color, float depth, int stencil);

	void BindWrite(const Rect& normalizedRect);

public:
	static RenderTexture* GetDefault();

	static RenderTexture* GetTemporary(RenderTextureFormat format, uint width, uint height);
	static void ReleaseTemporary(RenderTexture* texture);

protected:
	RenderTexture(void* d);
};

class SUEDE_API MRTRenderTexture : public RenderTexture {
	SUEDE_DEFINE_METATABLE_NAME(MRTRenderTexture)
	SUEDE_DECLARE_IMPLEMENTATION(MRTRenderTexture)

public:
	MRTRenderTexture();

public:
	bool AddColorTexture(TextureFormat format);
	Texture2D* GetColorTexture(uint index);
	uint GetColorTextureCount();
};

