#pragma once
#include <vector>
#include <string>

class ImageCodec {
public:
	static const void* Decode(const std::string& path, int& width, int& height);
	static bool Encode(int width, int height, std::vector<unsigned char>& data, const char* format);
};
