#include <Magick++.h>
#include "imagecodec.h"
#include "tools/debug.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

bool ImageCodec::Decode(const std::string& path, std::vector<unsigned char>& data, int& width, int& height) {
	try {
		Magick::Image image;
		Magick::Blob blob;

		image.read(path);
		image.write(&blob, "RGBA");
		width = image.columns();
		height = image.rows();

		// TODO: copy data out ?
		const unsigned char* bytes = (const unsigned char*)blob.data();
		data.resize(blob.length());
		std::copy(bytes, bytes + blob.length(), &data[0]);
	}
	catch (Magick::Error& err) {
		Debug::LogError("failed to decode image " + path + ": " + err.what());
		return false;
	}

	return true;
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
