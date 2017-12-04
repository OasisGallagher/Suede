#include <Magick++.h>

#include "math2.h"
#include "debug.h"
#include "image.h"

bool ImageCodec::Decode(std::vector<uchar>& data, int& width, int& height, const void* compressedData, uint length) {
	try {
		Magick::Blob blob(compressedData, length);
		Magick::Image image;
		image.read(blob);
		width = image.columns();
		height = image.rows();
		image.write(&blob, "RGBA");

		// TODO: copy data out ?
		const uchar* ptr = (const uchar*)blob.data();
		data.resize(blob.length());
		std::copy(ptr, ptr + blob.length(), &data[0]);
	}
	catch (Magick::Error& err) {
		Debug::LogError("failed to decode compressed data: %s.", err.what());
		return false;
	}

	return true;
}

bool ImageCodec::Decode(std::vector<uchar>& data, int& width, int& height, const std::string& path) {
	try {
		Magick::Image image;
		Magick::Blob blob;

		image.read(path);
		image.write(&blob, "RGBA");
		width = image.columns();
		height = image.rows();

		// TODO: copy data out ?
		const uchar* ptr = (const uchar*)blob.data();
		data.resize(blob.length());
		std::copy(ptr, ptr + blob.length(), &data[0]);
	}
	catch (Magick::Error& err) {
		Debug::LogError("failed to decode image \"%s\": %s.", path.c_str(), err.what());
		return false;
	}

	return true;
}

bool ImageCodec::Encode(int width, int height, std::vector<uchar>& data, const char* format) {
	Magick::Image image;
	try {
		image.read(width, height, "RGBA", Magick::CharPixel, &data[0]);
		image.flip();

		Magick::Blob blob;
		image.magick(format);
		image.write(&blob);

		uchar* ptr = (uchar*)blob.data();
		data.assign(ptr, ptr + blob.length());
	}
	catch (Magick::Error& err) {
		Debug::LogError("failed to encode image: %s.", err.what());
		return false;
	}

	return true;
}

bool AtlasMaker::Make(Atlas& atlas, const std::vector<Bitmap*>& bitmaps, int space) {
	int width, height;
	int columnCount = Calculate(width, height, bitmaps, space);

	// TODO: channel count.
	atlas.data.resize(width * height * 2);
	uchar* ptr = &atlas.data[0];
	int bottom = space;
	ptr += space * width * 2;

	for (int i = 0; i < bitmaps.size();) {
		int count = Math::Min((int)bitmaps.size() - i, columnCount);
		int rows = 0, offset = space * 2;

		for (int j = i; j < i + count; ++j) {
			const Bitmap* bitmap = bitmaps[j];
			float top = (bottom + bitmap->height) / (float)height;

			PasteBitmap(ptr + offset, bitmap, width);
			float left = offset / ((float)width * 2);
			float right = left + bitmap->width / (float)width;
			// order: left, bottom, right, top.
			atlas.coords[bitmap->id] = glm::vec4(left, bottom / (float)height, right, top);

			offset += (bitmap->width + space) * 2;
			rows = Math::Max(rows, bitmap->height);
		}

		ptr += (rows + space) * width * 2;
		bottom += (rows + space);
		i += count;
	}

	atlas.width = width;
	atlas.height = height;

	return true;
}

int AtlasMaker::Calculate(int& width, int& height, const std::vector<Bitmap*>& bitmaps, int space) {
	int columnCount = 16;
	int maxWidth = 0, maxHeight = 0;

	for (int i = 0; i < bitmaps.size();) {
		int count = Math::Min((int)bitmaps.size() - i, columnCount);
		int w = 0, h = 0;
		for (int j = i; j < i + count; ++j) {
			w += bitmaps[j]->width + space;
			h = Math::Max(h, bitmaps[j]->height);
		}

		maxWidth = Math::Max(maxWidth, w);
		maxHeight += h + space;
		i += count;
	}

	width = Math::NextPowerOfTwo(maxWidth + space);
	height = Math::NextPowerOfTwo(maxHeight + space);

	return columnCount;
}

void AtlasMaker::PasteBitmap(uchar* ptr, const Bitmap * bitmap, int stride) {
	for (int r = 0; r < bitmap->height; ++r) {
		for (int c = 0; c < bitmap->width; ++c) {
			uchar uch = bitmap->data[c + r * bitmap->width];
			// from left bottom.
			int r2 = bitmap->height - r - 1;
			ptr[2 * (c + r2 * stride)] = ptr[2 * (c + r2 * stride) + 1] = uch;
		}
	}
}
