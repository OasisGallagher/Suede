#pragma once
#include <map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "texture.h"

struct Bitmap {
	int id;

	int width;
	int height;
	std::vector<uchar> data;
	ColorFormat format;
};

class ImageCodec {
public:
	static void Initialize();
	static void Release();

	static bool Decode(Bitmap& bits, const std::string& path);
	static bool Decode(Bitmap& bits, const void* compressedData, uint length);
	static bool Encode(int width, int height, std::vector<uchar>& data, const char* format);
};

struct Atlas {
	int width;
	int height;

	std::vector<uchar> data;

	typedef std::map<uint, glm::vec4> CoordContainer;
	CoordContainer coords;
};

class AtlasMaker {
public:
	static bool Make(Atlas& atlas, const std::vector<Bitmap*>& bitmaps, int space);

private:
	static int Calculate(int& width, int& height, const std::vector<Bitmap*>& bitmaps, int space);
	static void PasteBitmap(uchar* ptr, const Bitmap* bitmap, int stride);
};
