#pragma once
#include <vector>
#include <string>

class ImageCodec {
public:
	static bool Decode(const std::string& path, std::vector<unsigned char>& data, int& width, int& height);
	static bool Encode(int width, int height, std::vector<unsigned char>& data, const char* format);
};
