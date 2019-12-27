#pragma once

#include "font.h"
#include "objectinternal.h"
#include "containers/ptrmap.h"
#include "internal/codec/image.h"

#define DECL_FT_TYPE(T)	typedef struct T ## Rec_* T;

DECL_FT_TYPE(FT_Face);
DECL_FT_TYPE(FT_Glyph);
DECL_FT_TYPE(FT_Library);
DECL_FT_TYPE(FT_BitmapGlyph);

#undef DECL_FT_TYPE

class FontInternal : public ObjectInternal {
public:
	FontInternal();
	~FontInternal();

public:
	bool Load(const std::string& path, int size);
	bool Require(Font* self, const std::wstring& str);

	uint GetFontSize() const { return size_; }
	Texture2D* GetTexture() const;

	std::string GetFamilyName() const;
	std::string GetStyleName() const;

	Material* GetMaterial() { return material_.get(); }

	bool GetCharacterInfo(wchar_t wch, CharacterInfo* info);

private:
	struct Glyph {
		RawImage rawImage;
	};

	typedef ptr_map<uint, Glyph> GlyphContainer;

private:
	void Destroy();
	bool Import(const std::string& path, int size);
	bool GetBitmapBits(wchar_t wch, RawImage* answer);

	void RebuildMaterial(Font* self);

private:
	ref_ptr<Material> material_;

	GlyphContainer glyphs_;
	Atlas::CoordContainer coords_;
	std::vector<RawImage*> rawImages_;

	int size_;
	std::string fname_;

	FT_Face face_;
	FT_Library library_;
};
