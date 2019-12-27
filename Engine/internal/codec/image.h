#pragma once
#include <map>
#include <vector>
#include <string>

#include "texture.h"

class RawImage {
public:
	int id = 0;

	uint width = 0;
	uint height = 0;

	uint alignment = 4;
	std::vector<uchar> pixels;

	TextureFormat textureFormat = TextureFormat::Rgba;
	ColorStreamFormat colorStreamFormat = ColorStreamFormat::Rgba;

public:
	void GetPixel(uchar* pixel, int x, int y) const;
	void GetPixelBilinear(uchar* pixel, float x, float y) const;
};

enum class ImageType {
	JPG,
	PNG,
};

struct FIBITMAP;
struct FIMEMORY;

class ImageCodec {
public:
	static bool Decode(RawImage& bits, const std::string& path);
	static bool Decode(RawImage& bits, const void* compressedData, uint length);
	static bool Encode(std::vector<uchar>& data, ImageType type, const RawImage& rawImage);

private:
	static bool SwapRedBlue(struct FIBITMAP* dib);

	static FIBITMAP* LoadDibFromMemory(FIMEMORY* stream);
	static FIBITMAP* LoadDibFromPath(const std::string &path);
	static FIBITMAP* LoadDibFromRawImage(const RawImage& rawImage);

	static bool EncodeDibTo(std::vector<uchar> &data, ImageType type, FIBITMAP* dib);

	static bool CopyTexelsTo(RawImage& bits, FIBITMAP* dib);
	static void CopyBitsFrom(FIBITMAP* dib, uint width, uint height, uint alignment, int bpp, const std::vector<uchar>& data);
};

struct Atlas {
	int width;
	int height;

	std::vector<uchar> data;

	typedef std::map<uint, Vector4> CoordContainer;
	CoordContainer coords;
};

class AtlasMaker {
public:
	/**
	 * @warning: RGBA format. alignment = 4.
	 */
	static bool Make(Atlas& atlas, const std::vector<RawImage*>& rawImages, uint space);

private:
	static uint Calculate(uint& width, uint& height, const std::vector<RawImage*>& rawImages, uint space);
	static void PasteTexels(uchar* ptr, const RawImage* rawImage, int stride);
};
