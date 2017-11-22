#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "variables.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "fontinternal.h"
#include "textureinternal.h"
#include "materialinternal.h"

FontInternal::FontInternal() 
	: ObjectInternal(ObjectTypeFont) , materialDirty_(false), size_(10), 
	face_(nullptr), library_(nullptr), atlasColumns_(16), atlasColumnCurrent_(0) {
}

FontInternal::~FontInternal() {
	Destroy();
}

bool FontInternal::Load(const std::string& fname, int size) {
	Destroy();
	return Import(fname, size);
}

bool FontInternal::Require(const std::string& str) {
	std::wstring wstr = String::MultiBytesToWideString(str);
	bool status = true;
	for (int i = 0; i < wstr.length(); ++i) {
		if (!bitmaps_.contains(wstr[i])) {
			FontBitmap* bitmap = bitmaps_[wstr[i]];
			status = GetBitmapBits(wstr[i], bitmap) && status;
			atlasData_.push_back(bitmap);
			materialDirty_ = true;
		}
	}

	return status;
}

Material FontInternal::GetMaterial() { 
	if (materialDirty_) { RebuildMaterial(); }
	return material_; 
}

bool FontInternal::Import(const std::string& fname, int size) {
	int err = 0;
	if ((err = FT_Init_FreeType(&library_)) != 0) {
		Debug::LogError(String::Format("failed to load font %s (%d)", fname.c_str(), err));
		return false;
	}

	if ((err = FT_New_Face(library_, fname.c_str(), 0, &face_)) != 0) {
		Debug::LogError(String::Format("failed to create font face for %s (%d)", fname.c_str(), err));
		return false;
	}

	FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
	FT_Set_Char_Size(face_, size << 6, size << 6, 96, 96);

	fname_ = fname;
	size_ = size;

	return true;
}

bool FontInternal::GetBitmapBits(wchar_t wch, FontBitmap* answer) {
	int err = 0;
	FT_Glyph glyph;
	if ((err = FT_Get_Glyph(face_->glyph, &glyph)) != 0) {
		Debug::LogError(String::Format("failed to get glyph for char %d (%d)", wch, err));
		return false;
	}

	if (answer == nullptr) {
		return true;
	}

	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
	const FT_Bitmap& bitmap = bitmapGlyph->bitmap;

	int width = Math::NextPowerOfTwo(bitmap.width);
	int height = Math::NextPowerOfTwo(bitmap.rows);
	Bytes& data = answer->data;
	size_t size = width * height;
	data.resize(size);
	std::copy(bitmap.buffer, bitmap.buffer + size, &data[0]);
	/*
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			unsigned char value = bitmap.buffer[j + bitmap.width * i];
			if (j < bitmap.width && i < bitmap.rows) {
				value = bitmap.buffer[j + bitmap.width * i];
			}

			data[2 * (j + i * width)] = data[2 * (j + i * width) + 1] = value;
		}
	}
	*/
	answer->width = width;
	answer->height = height;

	return true;
}

void FontInternal::CalculateAtlasSize(int& width, int& height, int space) {
	int maxWidth = 0, maxHeight = 0;

	for (int i = 0; i < atlasData_.size();) {
		int count = Math::Min((int)atlasData_.size() - i, atlasColumns_);
		int w = 0, h = 0;
		for (int j = i; j < i + count; ++j) {
			w += atlasData_[j]->width;
			h = Math::Max(h, atlasData_[j]->height);
		}

		maxWidth = Math::Max(maxWidth, w);
		maxHeight += h;
		i += count;
	}

	width = maxWidth;
	height = maxHeight;
}

void FontInternal::RebuildMaterial() {
	int width, height;
	int space = 2;
	CalculateAtlasSize(width, height, space);

	Bytes data(width * height);
	int offset = 0;

	for (int i = 0; i < atlasData_.size();) {
		int count = Math::Min((int)atlasData_.size() - i, atlasColumns_);
		int lineHeight = 0;
		int lineOffset = offset;
		int scannedWidth = 0;

		for (int j = i; j < i + count; ++j) {
			FontBitmap* bitmap = atlasData_[j];
			for (int r = 0; r < bitmap->height; ++r) {
				for (int c = 0; c < bitmap->width; ++c) {
					unsigned char pixel = bitmap->data[r * width + c];
					data[lineOffset + (width - scannedWidth) * r + c] = pixel;
				}
			}

			lineOffset += bitmap->width + space;
			scannedWidth += bitmap->width + space;
			lineHeight = Math::Max(lineHeight, bitmap->height);
		}

		offset += (lineHeight + space) * width;

		i += count;
	}

	Texture2D texture = CREATE_OBJECT(Texture2D);
	texture->Load(&data[0], ColorFormatIntensity, width, height);
	if (!material_) {
		material_ = CREATE_OBJECT(Material);
	}

	material_->SetTexture(Variables::mainTexture, texture);

	materialDirty_ = false;
}

void FontInternal::Destroy() {
	if (face_ != nullptr) {
		FT_Done_Face(face_);
		face_ = nullptr;
	}

	if (library_ != nullptr) {
		FT_Done_FreeType(library_);
	}

	fname_.clear();
}
