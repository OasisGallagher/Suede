#include <FreeImage.h>

#include "image.h"
#include "tools/math2.h"
#include "debug/debug.h"

static ColorStreamFormat BppToColorStreamFormat(uint bpp) {
	switch (bpp) {
	case 8:
	case 24:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		return ColorStreamFormatBgr;
#else
		return ColorStreamFormatRgb;
#endif
	case 32:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		return ColorStreamFormatBgra;
#else
		return ColorStreamFormatRgba;
#endif
	}

	Debug::LogError("invalid bbp number %d.", bpp);
	return ColorStreamFormatBgr;
}

static uint ColorStreamFormatToBpp(ColorStreamFormat format) {
	switch (format) {
	case ColorStreamFormatRgb:
	case ColorStreamFormatBgr:
		return 3 * 8;

	case ColorStreamFormatRgba:
	case ColorStreamFormatArgb:
	case ColorStreamFormatBgra:
		return 4 * 8;

	case ColorStreamFormatLuminanceAlpha:
		return 2 * 8;

	default:
		Debug::LogError("invalid color format %d.", format);
		break;
	}

	return 8;
}

bool ImageCodec::Decode(TexelMap& texelMap, const void* compressedData, uint length) {
	FIMEMORY* stream = FreeImage_OpenMemory((BYTE*)compressedData, length);
	FIBITMAP* dib = LoadDibFromMemory(stream);
	bool status = false;

	if (dib != nullptr) {
		FreeImage_FlipVertical(dib);
		status = CopyTexelsTo(texelMap, dib);
		FreeImage_Unload(dib);
	}

	FreeImage_CloseMemory(stream);
	return status;
}

bool ImageCodec::Decode(TexelMap& texelMap, const std::string& path) {
	FIBITMAP* dib = LoadDibFromPath(path);
	bool status = false;
	if (dib != nullptr) {
#ifdef _DEBUG
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
#endif // _DEBUG

		FreeImage_FlipVertical(dib);
		status = CopyTexelsTo(texelMap, dib);
		FreeImage_Unload(dib);
	}

	return status;
}

bool ImageCodec::Encode(std::vector<uchar>& data, ImageType type, const TexelMap& texelMap) {
	FIBITMAP* dib = LoadDibFromTexelMap(texelMap);
	bool status = EncodeDibTo(data, type, dib);
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

bool ImageCodec::CopyTexelsTo(TexelMap& texelMap, FIBITMAP* dib) {
	uint width = FreeImage_GetWidth(dib);
	uint height = FreeImage_GetHeight(dib);
	uint pitch = FreeImage_GetPitch(dib);
	uint bpp = FreeImage_GetBPP(dib);

	uint count = pitch * height;

	texelMap.data.resize(count);

	uchar* data = FreeImage_GetBits(dib);
	if (bpp != 8) {
		std::copy(data, data + count, &texelMap.data[0]);
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
				memcpy(&texelMap.data[(i * pitch + j) * 3], palette + k, 3 * sizeof(uchar));
            }
        }
	}

	texelMap.width = width;
	texelMap.height = height;
	texelMap.textureFormat = (bpp >= 32) ? TextureFormatRgba : TextureFormatRgb;
	texelMap.format = BppToColorStreamFormat(bpp);
	texelMap.alignment = 4;

	return true;
}

FIBITMAP* ImageCodec::LoadDibFromMemory(FIMEMORY* stream) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream);
	if (!FreeImage_FIFSupportsReading(fif)) {
		Debug::LogError("unsupported image format.");
		return nullptr;
	}


	FIBITMAP* dib = FreeImage_LoadFromMemory(fif, stream, 0);
	if (dib == nullptr) {
		Debug::LogError("failed to load image.");
		return nullptr;
	}

	return dib;
}

FIBITMAP* ImageCodec::LoadDibFromPath(const std::string &path) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.c_str());
	if (fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(path.c_str());
	}

	if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif)) {
		Debug::LogError("unsupported image format \"%s\".", path.c_str());
		return nullptr;
	}

	FIBITMAP* dib = FreeImage_Load(fif, path.c_str());
	if (dib == nullptr) {
		Debug::LogWarning("failed to load image \"%s\".", path.c_str());
		return nullptr;
	}

	return dib;
}

FIBITMAP* ImageCodec::LoadDibFromTexelMap(const TexelMap& texelMap) {
	if (texelMap.format != ColorStreamFormatRgb && texelMap.format != ColorStreamFormatRgba) {
		Debug::LogError("invalid format %d.", texelMap.format);
		return nullptr;
	}

	// TODO: 32 bbp jpg.
	BppType bpp = texelMap.format == ColorStreamFormatRgb ? BppType24 : BppType32;
	FIBITMAP* dib = FreeImage_Allocate(texelMap.width, texelMap.height, bpp);
	CopyBitsFrom(dib, texelMap.width, texelMap.height, bpp, texelMap.data);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
	SwapRedBlue(dib);
#endif

	return dib;
}

bool ImageCodec::EncodeDibTo(std::vector<uchar> &data, ImageType type, FIBITMAP* dib) {
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
	return status;
}

bool AtlasMaker::Make(Atlas& atlas, const std::vector<TexelMap*>& texelMaps, uint space) {
	if (texelMaps.empty()) {
		Debug::LogWarning("container is empty.");
		return false;
	}

	uint width, height;
	uint columnCount = Calculate(width, height, texelMaps, space);

	uint bpp = ColorStreamFormatToBpp(texelMaps.front()->format);
	atlas.data.resize(width * height * (bpp / 8));
	uchar* ptr = &atlas.data[0];
	int bottom = space;
	ptr += space * width * 2;

	for (int i = 0; i < texelMaps.size();) {
		uint count = Math::Min((uint)texelMaps.size() - i, columnCount);
		uint rows = 0, offset = space * 2;

		for (int j = i; j < i + count; ++j) {
			const TexelMap* texelMap = texelMaps[j];
			float top = (bottom + texelMap->height) / (float)height;

			PasteTexels(ptr + offset, texelMap, width);
			float left = offset / ((float)width * 2);
			float right = left + texelMap->width / (float)width;
			// order: left, bottom, right, top.
			atlas.coords[texelMap->id] = glm::vec4(left, bottom / (float)height, right, top);

			offset += (texelMap->width + space) * 2;
			rows = Math::Max(rows, texelMap->height);
		}

		ptr += (rows + space) * width * 2;
		bottom += (rows + space);
		i += count;
	}

	atlas.width = width;
	atlas.height = height;

	for (int i = 0; i < atlas.data.size(); ++i) {
		if (atlas.data[i] != 0) {
			//__debugbreak();
		}
	}

	return true;
}

uint AtlasMaker::Calculate(uint& width, uint& height, const std::vector<TexelMap*>& texelMaps, uint space) {
	uint columnCount = 16;
	uint maxWidth = 0, maxHeight = 0;

	for (int i = 0; i < texelMaps.size();) {
		uint count = Math::Min((uint)texelMaps.size() - i, columnCount);
		uint w = 0, h = 0;
		for (int j = i; j < i + count; ++j) {
			w += texelMaps[j]->width + space;
			h = Math::Max(h, texelMaps[j]->height);
		}

		maxWidth = Math::Max(maxWidth, w);
		maxHeight += h + space;
		i += count;
	}

	width = Math::NextPowerOfTwo(maxWidth + space);
	height = Math::NextPowerOfTwo(maxHeight + space);

	return columnCount;
}

void AtlasMaker::PasteTexels(uchar* ptr, const TexelMap* texelMap, int stride) {
	for (int r = 0; r < texelMap->height; ++r) {
		for (int c = 0; c < texelMap->width; ++c) {
			uchar uch = texelMap->data[c + r * texelMap->width];
			// from left bottom.
			int r2 = texelMap->height - r - 1;
			ptr[2 * (c + r2 * stride)] = ptr[2 * (c + r2 * stride) + 1] = uch;
		}
	}
}
