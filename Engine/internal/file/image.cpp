#include <Magick++.h>

#include "image.h"
#include "tools/math2.h"
#include "tools/debug.h"

bool ImageCodec::Decode(const std::string& path, Bytes& data, int& width, int& height) {
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

bool ImageCodec::Encode(int width, int height, Bytes& data, const char* format) {
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

bool AtlasMaker::Make(Atlas& atlas, const std::vector<Bitmap*>& bitmaps, int space) {
	int width, height;
	int columnCount = Calculate(width, height, bitmaps, space);

	// TODO: channel count.
	atlas.data.resize(width * height * 2);
	unsigned char* ptr = &atlas.data[0];
	float top = space / (float)height;
	ptr += space * width * 2 + space * 2;

	for (int i = 0; i < bitmaps.size();) {
		int count = Math::Min((int)bitmaps.size() - i, columnCount);
		int rows = 0, offset = 0;

		for (int j = i; j < i + count; ++j) {
			const Bitmap* bitmap = bitmaps[j];
			float bottom = top + bitmap->height / (float)height;

			PasteBitmap(ptr + offset, bitmap, width);
			float left = offset / ((float)width * 2);
			float right = left + bitmap->width / (float)width;
			atlas.coords[bitmap->id] = glm::vec4(left, bottom, right, top);

			offset += (bitmap->width + space) * 2;
			rows = Math::Max(rows, bitmap->height);
		}

		ptr += (rows + space) * width * 2;
		top += (rows + space) / (float)height;
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

void AtlasMaker::PasteBitmap(unsigned char * ptr, const Bitmap * bitmap, int stride) {
	for (int r = 0; r < bitmap->height; ++r) {
		for (int c = 0; c < bitmap->width; ++c) {
			unsigned char ch = bitmap->data[c + r * bitmap->width];
			// from left bottom.
			int r2 = bitmap->height - r - 1;
			ptr[2 * (c + r2 * stride)] = ptr[2 * (c + r2 * stride) + 1] = ch;
		}
	}
}
