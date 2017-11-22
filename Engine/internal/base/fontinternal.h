#pragma once

#include "font.h"
#include "objectinternal.h"
#include "internal/containers/ptrmap.h"

#define DECL_FT_TYPE(T)	typedef struct T ## Rec_* T;

DECL_FT_TYPE(FT_Face);
DECL_FT_TYPE(FT_Glyph);
DECL_FT_TYPE(FT_Library);
DECL_FT_TYPE(FT_BitmapGlyph);

class FontInternal : public IFont, public ObjectInternal {
public:
	FontInternal();
	~FontInternal();

public:
	virtual bool Load(const std::string& fname, int size);
	virtual bool Require(const std::string& str);
	virtual Material GetMaterial();

private:
	typedef std::vector<unsigned char> Bytes;
	struct FontBitmap {
		int width;
		int height;
		Bytes data;
		Glyph glyph;
	};

	typedef PtrMap<unsigned, FontBitmap> BitmapContainer;

private:
	void Destroy();
	bool Import(const std::string& fname, int size);
	bool GetBitmapBits(wchar_t wch, FontBitmap* answer);
	void CalculateAtlasSize(int& width, int& height, int space);
	void RebuildMaterial();

private:
	Material material_;
	bool materialDirty_;
	BitmapContainer bitmaps_;
	std::vector<FontBitmap*> atlasData_;

	int size_;
	std::string fname_;

	FT_Face face_;
	FT_Library library_;

	int atlasColumns_;
	int atlasColumnCurrent_;
};

#undef DECL_FT_TYPE
