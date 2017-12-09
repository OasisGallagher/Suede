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
	virtual bool Require(const std::wstring& str);
	virtual Material GetMaterial() { return material_; }

	virtual bool GetCharacterInfo(wchar_t wch, CharacterInfo* info);

private:
	struct Glyph {
		TexelMap texelMap;
	};

	typedef PtrMap<uint, Glyph> GlyphContainer;

private:
	void Destroy();
	bool Import(const std::string& path, int size);
	bool GetBitmapBits(wchar_t wch, TexelMap* answer);

	void RebuildMaterial();

private:
	Material material_;

	GlyphContainer glyphs_;
	Atlas::CoordContainer coords_;
	std::vector<TexelMap*> texelMaps_;

	int size_;
	std::string fname_;

	FT_Face face_;
	FT_Library library_;
};
