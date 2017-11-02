#include <Magick++.h>
#include "imagecodec.h"
#include "tools/debug.h"

const void* ImageCodec::Decode(const std::string& path, int& width, int& height) {
	Magick::Image image;
	static Magick::Blob blob;

	try {
		image.read(path.c_str());
		image.write(&blob, "RGBA");
		width = image.columns();
		height = image.rows();
	}
	catch (Magick::Error& err) {
		Debug::LogError("failed to decode image " + path + ": " + err.what());
		return nullptr;
	}

	return blob.data();
}

bool ImageCodec::Encode(int width, int height, std::vector<unsigned char>& data, const char* format) {
	Magick::Image image;
	try {
		image.read(width, height, "RGBA", Magick::CharPixel, &data[0]);
		image.flip();

		Magick::Blob blob;
		image.magick(format);
		image.write(&blob);
		unsigned char* ptr = (unsigned char*)blob.data();
		data.assign(ptr, ptr + blob.length());
	}
	catch (Magick::Error& err) {
		Debug::LogError(std::string("failed to encode image: ") + err.what());
		return false;
	}

	return true;
}
