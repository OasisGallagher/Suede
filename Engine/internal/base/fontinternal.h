#pragma once

#include "font.h"
#include "objectinternal.h"
#include "internal/file/image.h"
#include "internal/containers/ptrmap.h"

#define DECL_FT_TYPE(T)	typedef struct T ## Rec_* T;

DECL_FT_TYPE(FT_Face);
DECL_FT_TYPE(FT_Glyph);
DECL_FT_TYPE(FT_Library);
DECL_FT_TYPE(FT_BitmapGlyph);

#undef DECL_FT_TYPE

class FontInternal : public IFont, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Font)

public:
	FontInternal();
	~FontInternal();

public:
	virtual bool Load(const std::string& path, int size);
	virtual bool Require(const std::string& str);
	virtual Material GetMaterial();

private:
	struct Glyph {
		Bitmap bitmap;
		glm::vec2 lbTexCoord;
		glm::vec2 rtTexCoord;
	};

	typedef PtrMap<unsigned, Glyph> GlyphContainer;

private:
	void Destroy();
	bool Import(const std::string& path, int size);
	bool GetBitmapBits(wchar_t wch, Bitmap* answer);

	void RebuildMaterial();

private:
	Material material_;
	bool materialDirty_;

	Atlas atlas_;
	GlyphContainer glyphs_;
	std::vector<Bitmap*> bitmaps_;

	int size_;
	std::string fname_;

	FT_Face face_;
	FT_Library library_;
};
