#include "fontinternal.h"

#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "math/mathf.h"
#include "debug/debug.h"
#include "os/filesystem.h"
#include "builtinproperties.h"

#define U8_SPACE	(L' ')

Font::Font() : Object(new FontInternal) {}
bool Font::Load(const std::string& path) { return _suede_dptr()->Load(path); }
void Font::SetPixelHeight(float value) { _suede_dptr()->SetPixelHeight(value); }
float Font::GetPixelHeight() const { return _suede_dptr()->GetPixelHeight(); }
Material* Font::GetMaterial() { return _suede_dptr()->GetMaterial(); }
Texture2D* Font::GetAtlasTexture() { return _suede_dptr()->GetAtlasTexture(); }
bool Font::GetCharacterInfo(int codepoint, CharacterInfo* info) { return _suede_dptr()->GetCharacterInfo(codepoint, info); }
bool Font::RequestCharactersInTexture(const std::vector<int>& codepoints) { return _suede_dptr()->RequestCharactersInTexture(this, codepoints); }
int Font::GetAscent() const { return _suede_dptr()->GetAscent(); }
int Font::GetDecent() const { return _suede_dptr()->GetDecent(); }
int Font::GetLineGap() const { return _suede_dptr()->GetLineGap(); }
float Font::GetKerning(int codepoint0, int codepoint1) const { return _suede_dptr()->GetKerning(codepoint0, codepoint1); }

FontInternal::FontInternal() : ObjectInternal(ObjectType::Font) {
	fontinfo_ = new stbtt_fontinfo;
	atlasTexture_ = new Texture2D();
	atlasTexture_->SetName("FontAtlas");
	atlasTexture_->SetMinFilterMode(TextureMinFilterMode::Linear);
	atlasTexture_->SetMagFilterMode(TextureMagFilterMode::Linear);

	material_ = new Material();
	material_->SetShader(Shader::Find("builtin/unlit_texture"));
	material_->SetRenderQueue((int)RenderQueue::Transparent);

	// default font color.
	material_->SetColor(BuiltinProperties::MainColor, Color::white);
	material_->SetTexture(BuiltinProperties::MainTexture, atlasTexture_.get());
}

FontInternal::~FontInternal() {
	delete fontinfo_;
}

bool FontInternal::Load(const std::string& path) {
	if (!FileSystem::ReadAllBytes("resources/" + path, fontBytes_)) {
		return false;
	}

	int offset = stbtt_GetFontOffsetForIndex(fontBytes_.data(), 0);
	if (!stbtt_InitFont(fontinfo_, fontBytes_.data(), offset)) {
		Debug::LogError("failed to parse font data");
		return false;
	}

	scale_ = stbtt_ScaleForPixelHeight(fontinfo_, pixelHeight_);
	stbtt_GetFontVMetrics(fontinfo_, &ascent_, &descent_, &lineGap_);

	return true;
}

void FontInternal::SetPixelHeight(float value) {
	pixelHeight_ = value;
	scale_ = stbtt_ScaleForPixelHeight(fontinfo_, pixelHeight_);
}

void FontInternal::ClearAtlas() {
	glyphs_.clear();
	addedImages_.clear();
	cacheImages_.clear();

	atlas_ = Atlas();
	atlasMaker_.Reset();

	uchar black[] = { 0, 0, 0, 255 };
	atlasTexture_->Create(TextureFormat::Rgba, black, ColorStreamFormat::Rgba, 1, 1, 4, false);
}

bool FontInternal::GetCharacterInfo(int codepoint, CharacterInfo* info) {
	Glyph* glyph = nullptr;
	codepoint = FilterTextChar(codepoint);

	if (!glyphs_.get(codepoint, glyph)) {
		Debug::LogError("character %d does not in texture", codepoint);
		return false;
	}

	if (info != nullptr) {
		*info = glyph->characterInfo;
	}

	return true;
}

bool FontInternal::RequestCharactersInTexture(Font* self, const std::vector<int>& codepoints) {
	bool atlasContainsAllCodepoints = true;
	if (LoadCodepointsGlyphs(codepoints)) {
		atlasContainsAllCodepoints = AppendGlyphsToAtlas(codepoints);
		addedImages_.clear();

		self->materialRebuilt.raise();
	}

	return atlasContainsAllCodepoints;
}

bool FontInternal::LoadCodepointsGlyphs(const std::vector<int> &codepoints) {
	bool atlasDirty = false;
	for (int i = 0; i < codepoints.size(); ++i) {
		int cp = FilterTextChar(codepoints[i]);
		atlasDirty = LoadCodepointGlyph(cp) || atlasDirty;
	}

	return atlasDirty;
}

float FontInternal::GetKerning(int codepoint0, int codepoint1) const {
	int kern = stbtt_GetCodepointKernAdvance(fontinfo_, codepoint0, codepoint1);
	return kern * scale_;
}

int FontInternal::FilterTextChar(int wch) {
	char excludeArr[] = { '\r', '\n', '\t', '\v' };
	for (int i = 0; i < SUEDE_COUNTOF(excludeArr); ++i) {
		if (excludeArr[i] == wch) {
			return U8_SPACE;
		}
	}

	return wch;
}

bool FontInternal::LoadCodepointGlyph(int cp) {
	if (glyphs_.contains(cp)) { return false; }

	Glyph* glyph = glyphs_[cp];
	glyph->image.id = cp;

	int alternative = 0;
	int glyphIndex = stbtt_FindGlyphIndex(fontinfo_, cp);
	if (glyphIndex == 0) {
		if (LoadSystemCharGlyph(cp, glyph)) {
			addedImages_.push_back(&glyph->image);
			return true;
		}

		Debug::LogWarning("character 'u%x' not found, replace with 'u%x'", cp, U8_SPACE);

		// Use space instead.
		Glyph* spaceGlyph = nullptr;
		if (glyphs_.get(U8_SPACE, spaceGlyph) && spaceGlyph != nullptr) {
			*glyph = *spaceGlyph;
			return false;
		}

		// Load space glyph.
		alternative = U8_SPACE;
		glyphIndex = stbtt_FindGlyphIndex(fontinfo_, U8_SPACE);
	}

	LoadGlyph(glyphIndex, glyph);
	addedImages_.push_back(&glyph->image);

	if (alternative != 0) {
		*glyphs_[alternative] = *glyph;
	}

	return true;
}

void FontInternal::LoadGlyph(int glyphIndex, Glyph* glyph) {
	int c_x1, c_y1, c_x2, c_y2;
	stbtt_GetGlyphBitmapBox(fontinfo_, glyphIndex, scale_, scale_, &c_x1, &c_y1, &c_x2, &c_y2);

	int ax, lb;
	stbtt_GetGlyphHMetrics(fontinfo_, glyphIndex, &ax, &lb);

	glyph->characterInfo.advance = ax * scale_;
	glyph->characterInfo.minX = (float)c_x1;
	glyph->characterInfo.minY = -(float)c_y2;
	glyph->characterInfo.maxX = (float)c_x2;
	glyph->characterInfo.maxY = -(float)c_y1;

	LoadGlyphImage(glyphIndex, glyph->image);
}

void FontInternal::LoadGlyphImage(int glyphIndex, RawImage& image) {
	int width = 0, height = 0;

	std::vector<uchar>& buffer = image.pixels;
	uchar* pixels = stbtt_GetGlyphBitmap(fontinfo_, scale_, scale_, glyphIndex, &width, &height, nullptr, nullptr);
	if (pixels == nullptr) {
		buffer.resize(4, 255);
	}
	else {
		buffer.resize(width * height * 4, 255);

		// Alpha to RGBA
		for (int i = 0; i < width * height; ++i) {
			buffer[i * 4 + 3] = pixels[i];
		}

		free(pixels);
	}

	image.width = Mathf::Max(1, width);
	image.height = Mathf::Max(1, height);
	image.colorStreamFormat = ColorStreamFormat::Rgba;
	image.alignment = 4;
}

void FontInternal::SetGlyphCoords(int wch, const Vector4& coords) {
	Glyph* glyph = glyphs_[wch];

	glyph->characterInfo.uvLeftTop = Vector2(coords[0], coords[3]);
	glyph->characterInfo.uvLeftBottom = Vector2(coords[0], coords[1]);
	glyph->characterInfo.uvRightBottom = Vector2(coords[2], coords[1]);
	glyph->characterInfo.uvRightTop = Vector2(coords[2], coords[3]);
}

bool FontInternal::AppendGlyphsToAtlas(const std::vector<int>& codepoints) {
	cacheImages_.insert(cacheImages_.end(), addedImages_.begin(), addedImages_.end());
	if (cacheImages_.empty()) { return true; }

	switch (int status = atlasMaker_.Make(atlas_, cacheImages_, 2)) {
	case AtlasMaker::InvalidParameter:
		return false;

	case AtlasMaker::AtlasSizeOutOfRange:
		if ((status = RebuildAtlas(codepoints)) == AtlasMaker::AtlasSizeOutOfRange) {
			Debug::LogError("codepoints glyphs exceeds Max atlas size(%dx%d)", AtlasMaker::MaxAtlasWidth, AtlasMaker::MaxAtlasHeight);
			return false;
		}

		// Fall through

	case AtlasMaker::OK:
	case AtlasMaker::RebuiltOK:
		OnAtlasBuilt(status == AtlasMaker::RebuiltOK);
		break;
	}

	atlasTexture_->Create(TextureFormat::Rgba, atlas_.pixels.data(), ColorStreamFormat::Rgba, atlas_.width, atlas_.height, 4, false);
	return true;
}

void FontInternal::OnAtlasBuilt(bool atlasRebuilt) {
	if (atlasRebuilt) {
		// atlas change need re-set coords
		for (std::map<int, Vector4>::iterator ite = atlas_.coords.begin(); ite != atlas_.coords.end(); ++ite) {
			SetGlyphCoords(ite->first, ite->second);
		}
	}
	else {
		// atlas no change, only handle added images coords
		std::vector<RawImage*>::iterator iter;
		for (iter = addedImages_.begin(); iter != addedImages_.end(); ++iter) {
			std::map<int, Vector4>::iterator coordIter = atlas_.coords.find((*iter)->id);
			if (coordIter != atlas_.coords.end()) {
				SetGlyphCoords(coordIter->first, coordIter->second);
			}
		}
	}
}

int FontInternal::RebuildAtlas(const std::vector<int>& codepoints) {
	ClearAtlas();

	LoadCodepointsGlyphs(codepoints);
	return atlasMaker_.Make(atlas_, addedImages_, 2);
}
