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

class ImageCodec {
public:
	static bool Decode(RawImage& bits, const std::string& path);
	static bool Decode(RawImage& bits, const void* compressedData, uint length);

	static bool Encode(std::vector<uchar>& data, ImageType type, const RawImage& rawImage);

private:
	static void LoadRawImage(RawImage& rawImage, int width, int height, int channels, const void* data);
};

struct Atlas {
	uint width = 0;
	uint height = 0;
	uint alignment = 4;

	ColorStreamFormat colorFormat = ColorStreamFormat::Rgba;

	std::vector<uchar> pixels;
	std::map<int, Vector4> coords;
};

class AtlasMaker {
public:
	enum {
		MaxAtlasWidth = 4096,
		MaxAtlasHeight = 4096
	};

	enum {
		OK,
		RebuiltOK,

		InvalidParameter,
		AtlasSizeOutOfRange,
	};

	int Make(Atlas& atlas, const std::vector<RawImage*>& images, int space);
	void Reset() { cache_.Clear(); }

private:
	bool CalculateSize(std::vector<int>& columnArr, int& width, int& height, const std::vector<RawImage*>& images, int space);
	void CopyPixels(uchar* ptr, const RawImage* image, int stride, int channels);

private:
	struct {
		int imgCount = 0;

		int rowHeight = 0;
		int calWidth = 0, calHeight = 0;
		int totalWidth = 0, totalHeight = 0;

		std::vector<int> columnArr;

		uchar* ptr = nullptr;
		int top = 0, offset = 0, column = 0;

		void Clear() {
			columnArr.clear();
			ptr = nullptr;
			imgCount = totalWidth = totalHeight = calWidth = calHeight = column = top = rowHeight = offset = 0;
		}
	} cache_;
};