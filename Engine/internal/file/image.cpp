#include <FreeImage.h>

#include "math2.h"
#include "debug.h"
#include "image.h"

bool ImageCodec::Decode(Bitmap& bitmap, const void* compressedData, uint length) {
	FIMEMORY* stream = FreeImage_OpenMemory((BYTE*)compressedData, length);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream);
	FIBITMAP* dib = nullptr;
	if (!FreeImage_FIFSupportsReading(fif)) {
		Debug::LogError("unsupported image format.");
		FreeImage_CloseMemory(stream);
		return false;
	}

	dib = FreeImage_LoadFromMemory(fif, stream, 0);
	if (dib == nullptr) {
		FreeImage_CloseMemory(stream);
		Debug::LogError("failed to load image.");
		return false;
	}

	FreeImage_FlipVertical(dib);
	bool status = CopyBitsTo(bitmap, dib);

	FreeImage_Unload(dib);
	FreeImage_CloseMemory(stream);

	return status;
}

bool ImageCodec::Decode(Bitmap& bitmap, const std::string& path) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.c_str());
	if (fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(path.c_str());
	}

	if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif)) {
		Debug::LogError("unsupported image format \"%s\".", path.c_str());
		return false;
	}

	FIBITMAP* dib = FreeImage_Load(fif, path.c_str());
	if (dib == nullptr) {
		Debug::LogError("failed to load image \"%s\".", path.c_str());
		return false;
	}

	uint bpp = FreeImage_GetBPP(dib);
	if (bpp != 24 && bpp != 32) {
		__debugbreak();
	}

	int pitch = FreeImage_GetPitch(dib);
	int w = FreeImage_GetPitch(dib) / bpp * 8;
	int width = FreeImage_GetWidth(dib);
	if (w != width) {
		__debugbreak();
	}

	FreeImage_FlipVertical(dib);

	bool status = CopyBitsTo(bitmap, dib);

	FreeImage_Unload(dib);
	return status;
}

bool ImageCodec::Encode(int width, int height, std::vector<uchar>& data, BppType bpp, ImageType type) {
	FIBITMAP* dib = FreeImage_Allocate(width, height, bpp);

	CopyBitsFrom(dib, width, height, bpp, data);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
	SwapRedBlue(dib);
#endif

	FIMEMORY* stream = FreeImage_OpenMemory();

	bool status = false;
	if (FreeImage_SaveToMemory(type == ImageTypeJpg ? FIF_JPEG : FIF_PNG, dib, stream)) {
		ulong bytes = 0;
		uchar *buffer = nullptr;
		FreeImage_AcquireMemory(stream, &buffer, &bytes);
		data.assign(buffer, buffer + bytes);
		status = true;
	}
	else {
		Debug::LogError("failed to encode image.");
	}

	FreeImage_CloseMemory(stream);
	FreeImage_Unload(dib);

	return status;
}

void ImageCodec::CopyBitsFrom(FIBITMAP* dib, int width, int height, BppType bpp, const std::vector<uchar>& data) {
	uint srcStride = width * bpp / 8;
	uint destStride = FreeImage_GetPitch(dib);
	const uchar* src = &data[0];
	uchar* dest = FreeImage_GetBits(dib);
	if (srcStride == destStride) {
		memcpy(dest, src, srcStride * height);
	}
	else {
		for (int i = 0; i < height; i++) {
			memcpy(dest, src, srcStride);
			src += srcStride;
			dest += destStride;
		}
	}
}

bool ImageCodec::SwapRedBlue(FIBITMAP* dib) {
	const unsigned bpp = FreeImage_GetBPP(dib) / 8;
	if(bpp > 4 || bpp < 3) {
		Debug::LogError("failed to swap red blue, invalid bpp.");
		return false;
	}
	
	uint height = FreeImage_GetHeight(dib);
	uint pitch = FreeImage_GetPitch(dib);
	uint lineSize = FreeImage_GetLine(dib);
	
	uchar* line = FreeImage_GetBits(dib);
	for(int y = 0; y < height; ++y, line += pitch) {
		for(BYTE* pixel = line; pixel < line + lineSize ; pixel += bpp) {
			pixel[0] ^= pixel[2];
			pixel[2] ^= pixel[0];
			pixel[0] ^= pixel[2];
		}
	}
	
	return true;
}

bool ImageCodec::CopyBitsTo(Bitmap& bitmap, FIBITMAP* dib) {
	uint width = FreeImage_GetWidth(dib);
	uint height = FreeImage_GetHeight(dib);
	uint pitch = FreeImage_GetPitch(dib);
	uint bpp = FreeImage_GetBPP(dib);

	uint count = pitch * height;

	bitmap.data.resize(count);

	uchar* data = FreeImage_GetBits(dib);
	if (bpp != 8) {
		std::copy(data, data + count, &bitmap.data[0]);
	}
	else {
		RGBQUAD* palette = FreeImage_GetPalette(dib);
		if (palette == nullptr) {
			Debug::LogError("failed to get palette.");
			return false;
		}

		uint lineSize = FreeImage_GetLine(dib);
		for (int i = 0; i < height; ++i ) {  
            for (int j = 0; j < lineSize; ++j ) {  
                int k = data[i * pitch + j];  
				memcpy(&bitmap.data[(i * pitch + j) * 3], palette + k, 3 * sizeof(uchar));
            }
        }
	}

	bitmap.width = width;
	bitmap.height = height;
	bitmap.format = BppToColorFormat(bpp);
	bitmap.alignment = 4;

	return true;
}

ColorFormat ImageCodec::BppToColorFormat(uint bbp) {
	switch (bbp) {
		case 8:
		case 24:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
			return ColorFormatBgr;
#else
			return ColorFormatRgb;
#endif
		case 32:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
			return ColorFormatBgra;
#else
			return ColorFormatRgba;
#endif
	}

	Debug::LogError("invalid bbp number %d.", bbp);
	return ColorFormatBgr;
}

bool AtlasMaker::Make(Atlas& atlas, const std::vector<Bitmap*>& bitmaps, uint space) {
	uint width, height;
	uint columnCount = Calculate(width, height, bitmaps, space);

	// TODO: channel count.
	atlas.data.resize(width * height * 2);
	uchar* ptr = &atlas.data[0];
	int bottom = space;
	ptr += space * width * 2;

	for (int i = 0; i < bitmaps.size();) {
		uint count = Math::Min((uint)bitmaps.size() - i, columnCount);
		uint rows = 0, offset = space * 2;

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

uint AtlasMaker::Calculate(uint& width, uint& height, const std::vector<Bitmap*>& bitmaps, uint space) {
	uint columnCount = 16;
	uint maxWidth = 0, maxHeight = 0;

	for (int i = 0; i < bitmaps.size();) {
		uint count = Math::Min((uint)bitmaps.size() - i, columnCount);
		uint w = 0, h = 0;
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
