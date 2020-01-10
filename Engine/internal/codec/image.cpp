#include "image.h"
#include "math/mathf.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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
	int width = 0, height = 0, channels = 0;
	stbi_uc* data = stbi_load_from_memory((const stbi_uc*)compressedData, length, &width, &height, &channels, 0);
	if (data == nullptr) {
		Debug::LogError("failed to load image: %s", stbi_failure_reason());
		return false;
	}

	LoadRawImage(rawImage, width, height, channels, data);
	STBIW_FREE(data);
	return true;
}

bool ImageCodec::Decode(RawImage& rawImage, const std::string& path) {
	int width = 0, height = 0, channels = 0;
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (data == nullptr) {
		Debug::LogError("failed to load \"%s\": %s", path.c_str(), stbi_failure_reason());
		return false;
	}

	if (channels != 3 && channels != 4) {
		Debug::LogError("failed to load \"%s\", invalid channals %d", path.c_str(), channels);
		channels = -1;
	}
	else {
		LoadRawImage(rawImage, width, height, channels, data);
	}

	STBIW_FREE(data);
	return channels > 0;
}

void ImageCodec::LoadRawImage(RawImage &rawImage, int width, int height, int channels, const void* data) {
	rawImage.alignment = 4;
	rawImage.textureFormat = (channels == 3) ? TextureFormat::Rgb : TextureFormat::Rgba;
	rawImage.colorStreamFormat = (channels == 3) ? ColorStreamFormat::Rgb : ColorStreamFormat::Rgba;
	rawImage.width = width;
	rawImage.height = height;

	// The value in the Pitch member describes the surface's memory pitch, also called stride. 
	// Pitch is the distance, in bytes, between two memory addresses that represent the beginning 
	// of one bitmap line and the beginning of the next bitmap line.
	int pitch = Mathf::RoundUpToPowerOfTwo(width, 4) * channels;
	rawImage.pixels.resize(pitch * height);

	uchar* dest = rawImage.pixels.data();
	int widthInBytes = width * channels;

	for (int i = 0; i < height; ++i) {
		memcpy(dest, (const stbi_uc*)data + (widthInBytes * i), widthInBytes);
		dest += pitch;
	}
}

bool ImageCodec::Encode(std::vector<uchar>& data, ImageType type, const RawImage& rawImage) {
	int length = 0;
	int channels = ColorStreamChannels(rawImage.colorStreamFormat);
	stbi_uc* memory = stbi_write_png_to_mem(
		rawImage.pixels.data(), 
		Mathf::RoundUpToPowerOfTwo(rawImage.width, rawImage.alignment) * channels,
		rawImage.width, rawImage.height, ColorStreamChannels(rawImage.colorStreamFormat), &length
	);

	if (memory == nullptr) {
		Debug::LogError("failed to write image");
		return false;
	}

	data.assign(memory, memory + length);
	STBIW_FREE(memory);

	return true;
}

int AtlasMaker::Make(Atlas& atlas, const std::vector<RawImage*>& images, int space) {
	if (images.empty()) {
		Debug::LogError("atlas container is empty");
		return InvalidParameter;
	}

	atlas.colorFormat = images.front()->colorStreamFormat;
	int channels = ColorStreamChannels(atlas.colorFormat);

	int oldWidth = atlas.width, oldHeight = atlas.height;
	int width, height;
	std::vector<int> oldCacheArr = cache_.columnArr;
	std::vector<int> columnArr;

	if (!CalculateSize(columnArr, width, height, images, space)) {
		return AtlasSizeOutOfRange;
	}

	width = Mathf::RoundUpToPowerOfTwo(width, 4);
	height = Mathf::RoundUpToPowerOfTwo(height, 4);

	int status = OK;

	if (width > oldWidth || height > oldHeight) {
		// clear cache£¬rebuild atlas
		cache_.imgCount = 0;
		oldCacheArr.resize(0);
		atlas = Atlas();
		atlas.pixels.resize(width * height * channels);
		status = RebuiltOK;
	}

	int top = 0;
	uchar* ptr = nullptr;
	if (cache_.imgCount < 1) {
		ptr = &atlas.pixels[0];
		ptr += space * width * channels;
		top = space;
	}
	else {
		ptr = cache_.ptr;
		top = cache_.top;
	}

	int rowIndex = Mathf::Max(0, (int)oldCacheArr.size() - 1);
	for (int i = cache_.imgCount; i < images.size();) {
		bool atCacheRestRow = cache_.imgCount > 0 && rowIndex == oldCacheArr.size() - 1;
		uint rowHeight = atCacheRestRow ? cache_.rowHeight : 0;
		int offset = atCacheRestRow ? cache_.offset : space * channels;

		int restColumn = atCacheRestRow ? oldCacheArr.at(rowIndex) : 0;
		int delCount = columnArr.at(rowIndex) - restColumn;

		for (int j = i; j < i + delCount; ++j) {
			const RawImage* image = images[j];
			float bottom = (top + image->height) / (float)height;

			CopyPixels(ptr + offset, image, width, channels);
			float left = offset / ((float)width * channels);
			float right = left + image->width / (float)width;

			// order: left, bottom, right, top.
			atlas.coords[image->id] = Vector4(left, bottom, right, top / (float)height);

			offset += (image->width + space) * channels;
			rowHeight = Mathf::Max(rowHeight, image->height);

			cache_.top = top;
			cache_.ptr = ptr;
		}
		ptr += (rowHeight + space) * width * channels;
		top += (rowHeight + space);
		rowIndex++;
		i += delCount;

		cache_.offset = offset;
		cache_.rowHeight = rowHeight;
	}
	cache_.imgCount = images.size();
	atlas.width = width;
	atlas.height = height;

	atlas.alignment = 4;

	return status;
}

bool AtlasMaker::CalculateSize(std::vector<int>& columnArr, int& width, int& height, const std::vector<RawImage*>& images, int space) {
	columnArr = cache_.columnArr;
	if (columnArr.size() > 0) {
		columnArr.pop_back();
	}

	uint calWidth = cache_.calWidth, calHeight = cache_.calHeight;
	uint totalWidth = cache_.totalWidth, totalHeight = cache_.totalHeight - calHeight;
	int column = cache_.column;
	for (int j = cache_.imgCount; j < images.size(); j++) {
		if (calWidth + images[j]->width + space > MaxAtlasWidth) {
			totalHeight += calHeight;
			calHeight = 0;
			calWidth = 0;

			columnArr.push_back(column);
			column = 0;
		}

		calWidth += images[j]->width + space;
		calHeight = Mathf::Max(calHeight, images[j]->height + space);

		totalWidth = Mathf::Max(calWidth, totalWidth);
		column++;
	}

	totalHeight += calHeight;
	columnArr.push_back(column);

	if (totalHeight > MaxAtlasHeight) {
		return false;
	}

	cache_.totalWidth = totalWidth;
	cache_.totalHeight = totalHeight;
	cache_.calWidth = calWidth;
	cache_.calHeight = calHeight;
	cache_.column = column;
	cache_.columnArr = columnArr;

	width = totalWidth;
	height = totalHeight;

	return true;
}

void AtlasMaker::CopyPixels(uchar* ptr, const RawImage* image, int stride, int channels) {
	for (int r = 0; r < image->height; ++r) {
		for (int c = 0; c < image->width; ++c) {
			memcpy(ptr + channels * (c + r * stride),
				image->pixels.data() + channels * (c + r * image->width),
				channels
			);
		}
	}
}
