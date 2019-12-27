#include <FreeImage.h>

#include "image.h"
#include "math/mathf.h"
#include "debug/debug.h"

void RawImage::GetPixelBilinear(uchar* pixel, float x, float y) const {
	int px = (int)x;
	int py = (int)y;
	const int nbytes = ColorStreamBytes(colorStreamFormat);
	const uchar* p0 = pixels.data() + (px + py * width) * nbytes;

	const uchar* p1 = p0 + (0 + 0 * width) * nbytes;
	const uchar* p2 = ((px + 1 < width) ? p0 + (1 + 0 * width) * nbytes : p1);
	const uchar* p3 = ((py + 1 < height) ? p0 + (0 + 1 * width) * nbytes : p1);
	const uchar* p4 = ((px + 1 < width && py + 1 < height) ? p0 + (1 + 1 * width) * nbytes : p1);

	// Calculate the weights for each pixel
	float fx = x - px;
	float fy = y - py;
	float fx1 = 1.0f - fx;
	float fy1 = 1.0f - fy;

	float w1 = fx1 * fy1;
	float w2 = fx  * fy1;
	float w3 = fx1 * fy;
	float w4 = fx  * fy;

	// Calculate the weighted sum of pixels (for each color channel)
	for (int i = 0; i < nbytes; ++i) {
		pixel[i] = uchar(*(p1 + i) * w1 + *(p2 + i) * w2 + *(p3 + i) * w3 + *(p4 + i) * w4);
	}
}

void RawImage::GetPixel(uchar* pixel, int x, int y) const {
	int nbytes = ColorStreamBytes(colorStreamFormat);
	memcpy(pixel, pixels.data() + (y * width + x) * nbytes, nbytes);
}

bool ImageCodec::Decode(RawImage& rawImage, const void* compressedData, uint length) {
	FIMEMORY* stream = FreeImage_OpenMemory((uchar*)compressedData, length);
	FIBITMAP* dib = LoadDibFromMemory(stream);
	bool status = false;

	if (dib != nullptr) {
		FreeImage_FlipVertical(dib);
		status = CopyTexelsTo(rawImage, dib);
		FreeImage_Unload(dib);
	}

	FreeImage_CloseMemory(stream);
	return status;
}

bool ImageCodec::Decode(RawImage& rawImage, const std::string& path) {
	FIBITMAP* dib = LoadDibFromPath(path);
	bool status = false;
	if (dib != nullptr) {
		uint bpp = FreeImage_GetBPP(dib);
		if (bpp != 24 && bpp != 32) {
			Debug::Break();
		}

		/**	https://msdn.microsoft.com/en-us/library/widgets/desktop/bb206357(v=vs.85).aspx
		int pitch = FreeImage_GetPitch(dib);
		int w = FreeImage_GetPitch(dib) / bpp * 8;
		int width = FreeImage_GetWidth(dib);
		if (w != width) {
			Debug::Break();
		}
		*/

		FreeImage_FlipVertical(dib);
		status = CopyTexelsTo(rawImage, dib);
		FreeImage_Unload(dib);
	}

	return status;
}

bool ImageCodec::Encode(std::vector<uchar>& data, ImageType type, const RawImage& rawImage) {
	FIBITMAP* dib = LoadDibFromRawImage(rawImage);
	bool status = EncodeDibTo(data, type, dib);
	FreeImage_Unload(dib);

	return status;
}

void ImageCodec::CopyBitsFrom(FIBITMAP* dib, uint width, uint height, uint alignment, int bpp, const std::vector<uchar>& data) {
	uint srcStride = Mathf::RoundUpToPowerOfTwo(width * bpp / 8, alignment);
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
		for(uchar* pixel = line; pixel < line + lineSize ; pixel += bpp) {
			pixel[0] ^= pixel[2];
			pixel[2] ^= pixel[0];
			pixel[0] ^= pixel[2];
		}
	}
	
	return true;
}

bool ImageCodec::CopyTexelsTo(RawImage& rawImage, FIBITMAP* dib) {
	uint width = FreeImage_GetWidth(dib);
	uint height = FreeImage_GetHeight(dib);
	uint pitch = FreeImage_GetPitch(dib);
	uint bpp = FreeImage_GetBPP(dib);

	uint count = pitch * height;

	rawImage.pixels.resize(count);

	uchar* data = FreeImage_GetBits(dib);
	if (bpp != 8) {
		std::copy(data, data + count, rawImage.pixels.data());
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
				memcpy(&rawImage.pixels[(i * pitch + j) * 3], palette + k, 3 * sizeof(uchar));
            }
        }
	}

	ColorStreamFormat colorStreamFormat = ColorStreamFormat::Rgba;
	switch (bpp) {
	case 8:
	case 24:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		colorStreamFormat = ColorStreamFormat::Bgr;
#else
		colorStreamFormat = ColorStreamFormat::Rgb;
#endif
		break;

	case 32:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		colorStreamFormat = ColorStreamFormat::Bgra;
#else
		colorStreamFormat = ColorStreamFormat::Rgba;
#endif
		break;
	}

	rawImage.width = width;
	rawImage.height = height;
	rawImage.textureFormat = (bpp >= 32) ? TextureFormat::Rgba : TextureFormat::Rgb;
	rawImage.colorStreamFormat = colorStreamFormat;
	rawImage.alignment = 4;

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

FIBITMAP* ImageCodec::LoadDibFromRawImage(const RawImage& rawImage) {
	if (rawImage.colorStreamFormat != ColorStreamFormat::Rgb && rawImage.colorStreamFormat != ColorStreamFormat::Rgba) {
		Debug::LogError("invalid format %d.", rawImage.colorStreamFormat);
		return nullptr;
	}

	int bpp = ColorStreamBytes(rawImage.colorStreamFormat) * 8;
	FIBITMAP* dib = FreeImage_Allocate(rawImage.width, rawImage.height, bpp);
	CopyBitsFrom(dib, rawImage.width, rawImage.height, rawImage.alignment, bpp, rawImage.pixels);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
	SwapRedBlue(dib);
#endif

	return dib;
}

bool ImageCodec::EncodeDibTo(std::vector<uchar> &data, ImageType type, FIBITMAP* dib) {
	FIMEMORY* stream = FreeImage_OpenMemory();

	bool status = false;
	if (FreeImage_SaveToMemory(type == ImageType::JPG ? FIF_JPEG : FIF_PNG, dib, stream)) {
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

bool AtlasMaker::Make(Atlas& atlas, const std::vector<RawImage*>& rawImages, uint space) {
	if (rawImages.empty()) {
		Debug::LogWarning("container is empty.");
		return false;
	}

	uint width, height;
	uint columnCount = Calculate(width, height, rawImages, space);

	uint bpp = ColorStreamBytes(rawImages.front()->colorStreamFormat) * 8;
	atlas.data.resize(width * height * (bpp / 8));
	uchar* ptr = atlas.data.data();
	int bottom = space;
	ptr += space * width * 2;

	for (int i = 0; i < rawImages.size();) {
		uint count = Mathf::Min((uint)rawImages.size() - i, columnCount);
		uint rows = 0, offset = space * 2;

		for (int j = i; j < i + count; ++j) {
			const RawImage* rawImage = rawImages[j];
			float top = (bottom + rawImage->height) / (float)height;

			PasteTexels(ptr + offset, rawImage, width);
			float left = offset / ((float)width * 2);
			float right = left + rawImage->width / (float)width;
			// order: left, bottom, right, top.
			atlas.coords[rawImage->id] = Vector4(left, bottom / (float)height, right, top);

			offset += (rawImage->width + space) * 2;
			rows = Mathf::Max(rows, rawImage->height);
		}

		ptr += (rows + space) * width * 2;
		bottom += (rows + space);
		i += count;
	}

	atlas.width = width;
	atlas.height = height;

	return true;
}

uint AtlasMaker::Calculate(uint& width, uint& height, const std::vector<RawImage*>& rawImages, uint space) {
	uint columnCount = 16;
	uint maxWidth = 0, maxHeight = 0;

	for (int i = 0; i < rawImages.size();) {
		uint count = Mathf::Min((uint)rawImages.size() - i, columnCount);
		uint w = 0, h = 0;
		for (int j = i; j < i + count; ++j) {
			w += rawImages[j]->width + space;
			h = Mathf::Max(h, rawImages[j]->height);
		}

		maxWidth = Mathf::Max(maxWidth, w);
		maxHeight += h + space;
		i += count;
	}

	width = Mathf::NextPowerOfTwo(maxWidth + space);
	height = Mathf::NextPowerOfTwo(maxHeight + space);

	return columnCount;
}

void AtlasMaker::PasteTexels(uchar* ptr, const RawImage* rawImage, int stride) {
	for (int r = 0; r < rawImage->height; ++r) {
		for (int c = 0; c < rawImage->width; ++c) {
			uchar uch = rawImage->pixels[c + r * rawImage->width];
			// from left bottom.
			int r2 = rawImage->height - r - 1;
			ptr[2 * (c + r2 * stride)] = ptr[2 * (c + r2 * stride) + 1] = uch;
		}
	}
}
