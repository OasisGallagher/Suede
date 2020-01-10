#pragma once

#include "font.h"
#include "objectinternal.h"
#include "containers/ptrmap.h"
#include "internal/codec/image.h"

#define DEFAULT_FONT_PIXEL_HEIGHT		(18.f)

class FontInternal : public ObjectInternal {
public:
	FontInternal();
	~FontInternal();

public:
	bool Load(const std::string& path);

	void SetPixelHeight(float value);
	float GetPixelHeight() const { return pixelHeight_; }

	Material* GetMaterial() { return material_.get(); }
	Texture2D* GetAtlasTexture() { return atlasTexture_.get(); }

	bool GetCharacterInfo(int codepoint, CharacterInfo* info);
	bool RequestCharactersInTexture(Font* self, const std::vector<int>& codepoints);

	int GetAscent() const { return ascent_; }
	int GetDecent() const { return descent_; }
	int GetLineGap() const { return lineGap_; }
	float GetKerning(int codepoint0, int codepoint1) const;

protected:
	struct Glyph {
		RawImage image;
		CharacterInfo characterInfo;
	};

	virtual bool LoadSystemCharGlyph(int cp, Glyph* glyph) { return false; }

private:
	void ClearAtlas();
	int RebuildAtlas(const std::vector<int>& codepoints);

	bool AppendGlyphsToAtlas(const std::vector<int>& codepoints);

	void OnAtlasBuilt(bool atlasRebuilt);

	bool LoadCodepointGlyph(int cp);
	bool LoadCodepointsGlyphs(const std::vector<int> &codepoints);

	void LoadGlyph(int glyphIndex, Glyph* glyph);
	void LoadGlyphImage(int glyphIndex, RawImage& image);

	void SetGlyphCoords(int wch, const Vector4& coords);

	int FilterTextChar(int cp);

private:
	typedef ptr_map<int, Glyph> GlyphContainer;

	float scale_ = 1.f;

	int ascent_ = 0, descent_ = 0, lineGap_ = 0;

	float pixelHeight_ = DEFAULT_FONT_PIXEL_HEIGHT;

	std::vector<uchar> fontBytes_;
	struct stbtt_fontinfo* fontinfo_ = nullptr;

	GlyphContainer glyphs_;

	ref_ptr<Material> material_;
	ref_ptr<Texture2D> atlasTexture_;

	std::vector<RawImage*> addedImages_;

	std::vector<RawImage*> cacheImages_;

	Atlas atlas_;
	AtlasMaker atlasMaker_;
};