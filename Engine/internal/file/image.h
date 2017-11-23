#pragma once
#include <map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "texture.h"

typedef unsigned char Channel;
typedef std::vector<unsigned char> Bytes;

struct Bitmap {
	int id;

	int width;
	int height;
	Bytes data;
	ColorFormat format;
};

class ImageCodec {
public:
	static bool Decode(const std::string& path, Bytes& data, int& width, int& height);
	static bool Encode(int width, int height, Bytes& data, const char* format);
};

struct Atlas {
	int width;
	int height;

	Bytes data;

	typedef std::map<unsigned, glm::vec4> CoordContainer;
	CoordContainer coords;
};

class AtlasMaker {
public:
	static bool Make(Atlas& atlas, const std::vector<Bitmap*>& bitmaps, int space);

private:
	static int Calculate(int& width, int& height, const std::vector<Bitmap*>& bitmaps, int space);
	static void PasteBitmap(unsigned char* ptr, const Bitmap* bitmap, int stride);
};
