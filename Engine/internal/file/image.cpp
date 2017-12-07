#include <Magick++.h>
#include <FreeImage.h>

#include "math2.h"
#include "debug.h"
#include "image.h"

void ImageCodec::Initialize() {
	FreeImage_Initialise();
}

void ImageCodec::Release() {
	FreeImage_DeInitialise();
}

bool ImageCodec::Decode(Bitmap& bits, const void* compressedData, uint length) {
	FIMEMORY* stream = FreeImage_OpenMemory((BYTE*)compressedData, length);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream);
	FIBITMAP* bitmap = nullptr;
	if (FreeImage_FIFSupportsReading(fif)) {
		bitmap = FreeImage_LoadFromMemory(fif, stream, 0);
	}

	if (bitmap == nullptr) {
		FreeImage_CloseMemory(stream);
		Debug::LogError("failed to decode image.");
		return false;
	}

	FreeImage_FlipVertical(bitmap);

	uchar* data = FreeImage_GetBits(bitmap);
	uint width = FreeImage_GetWidth(bitmap);
	uint height = FreeImage_GetHeight(bitmap);
	uint channels = FreeImage_GetLine(bitmap) / width;
	uint count = width * height * channels;

	bits.data.resize(count);

	std::copy(data, data + count, &bits.data[0]);

	FreeImage_Unload(bitmap);
	FreeImage_CloseMemory(stream);

	bits.width = width;
	bits.height = height;
	bits.format = ColorFormatBgr;

	return true;
}

bool ImageCodec::Decode(Bitmap& bits, const std::string& path) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.c_str());
	if (fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(path.c_str());
	}

	FIBITMAP *bitmap = NULL;
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		bitmap = FreeImage_Load(fif, path.c_str());
	}

	uint bbp = FreeImage_GetBPP(bitmap);
	if (FreeImage_GetBPP(bitmap) != 32) {
		FIBITMAP* hOldImage = bitmap;
		//hImage = FreeImage_ConvertTo32Bits(hOldImage);
		//FreeImage_Unload(hOldImage);
	}

	FreeImage_FlipVertical(bitmap);

	uint width = FreeImage_GetWidth(bitmap);
	uint height = FreeImage_GetHeight(bitmap);
	uint channels = FreeImage_GetLine(bitmap) / width;
	uint count = width * height * channels;

	bits.data.resize(count);

	uchar* data = FreeImage_GetBits(bitmap);
	std::copy(data, data + count, &bits.data[0]);

	FreeImage_Unload(bitmap);

	bits.width = width;
	bits.height = height;
	bits.format = ColorFormatBgr;

	return true;
	/*
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

	return true;*/
}

bool ImageCodec::Encode(int width, int height, std::vector<uchar>& data, const char* format) {
	const uint bpp = 24;
	FIBITMAP* bitmap = FreeImage_Allocate(width, height, bpp);
	for(int y =0 ;y<FreeImage_GetHeight(bitmap);y++)
    {
        BYTE *bits =FreeImage_GetScanLine(bitmap,y);
        for(int x =0 ;x<FreeImage_GetWidth(bitmap);x++)
        {
            bits[0] = data[(y*width +x)*4+0];
            bits[1] = data[(y*width +x)*4+1];
            bits[2] = data[(y*width +x)*4+2];
            ///bits[3] = 255;
            bits += bpp / 8;
        }
    }

	FIMEMORY* stream = FreeImage_OpenMemory();

	auto t = FreeImage_GetImageType(bitmap);
	bool ok = !!FreeImage_SaveToMemory(FIF_JPEG, bitmap, stream);
	
	BYTE *mem_buffer = NULL;
    DWORD size_in_bytes = 0;
    FreeImage_AcquireMemory(stream, &mem_buffer, &size_in_bytes);

	data.assign(mem_buffer, mem_buffer + size_in_bytes);

	return true;
// 
// 	FIMEMORY* stream = FreeImage_OpenMemory((BYTE*)data.data(), data.size());
// 	//FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream);
// 	FIBITMAP* bitmap = nullptr;
// 	if (FreeImage_FIFSupportsReading(FIF_RAW)) {
// 		bitmap = FreeImage_LoadFromMemory(FIF_RAW, stream, 0);
// 	}
// 	Magick::Image image;
// 	try {
// 		image.read(width, height, "RGBA", Magick::CharPixel, &data[0]);
// 		image.flip();
// 
// 		Magick::Blob blob;
// 		image.magick(format);
// 		image.write(&blob);
// 
// 		uchar* ptr = (uchar*)blob.data();
// 		data.assign(ptr, ptr + blob.length());
// 	}
// 	catch (Magick::Error& err) {
// 		Debug::LogError("failed to encode image: %s.", err.what());
// 		return false;
// 	}
// 
// 	return true;
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
