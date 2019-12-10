#pragma once
#include <map>
#include <vector>
#include <string>

#include "texture.h"

struct TexelMap {
	int id;

	uint width;
	uint height;
	uint alignment;
	TextureFormat textureFormat;
	ColorStreamFormat colorStreamFormat;
	std::vector<uchar> data;
};

enum ImageType {
	ImageTypeJPG,
	ImageTypePNG,
};

enum BPPType {
	BPPType24 = 24,
	BPPType32 = 32,
};

struct FIBITMAP;
struct FIMEMORY;

class ImageCodec {
public:
	static bool Decode(TexelMap& bits, const std::string& path);
	static bool Decode(TexelMap& bits, const void* compressedData, uint length);
	static bool Encode(std::vector<uchar>& data, ImageType type, const TexelMap& texelMap);

private:
	static bool SwapRedBlue(struct FIBITMAP* dib);

	static FIBITMAP* LoadDibFromMemory(FIMEMORY* stream);
	static FIBITMAP* LoadDibFromPath(const std::string &path);
	static FIBITMAP* LoadDibFromTexelMap(const TexelMap& texelMap);

	static bool EncodeDibTo(std::vector<uchar> &data, ImageType type, FIBITMAP* dib);

	static bool CopyTexelsTo(TexelMap& bits, FIBITMAP* dib);
	static void CopyBitsFrom(FIBITMAP* dib, uint width, uint height, uint alignment, BPPType bpp, const std::vector<uchar>& data);
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
	static bool Make(Atlas& atlas, const std::vector<TexelMap*>& texelMaps, uint space);

private:
	static uint Calculate(uint& width, uint& height, const std::vector<TexelMap*>& texelMaps, uint space);
	static void PasteTexels(uchar* ptr, const TexelMap* texelMap, int stride);
};
