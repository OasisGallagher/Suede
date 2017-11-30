#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "variables.h"
#include "tools/path.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "fontinternal.h"
#include "shaderinternal.h"
#include "textureinternal.h"
#include "materialinternal.h"
#include "internal/file/image.h"
#include "internal/resources/resources.h"

FontInternal::FontInternal() 
	: ObjectInternal(ObjectTypeFont) ,size_(10), face_(nullptr), library_(nullptr) {
	material_ = CREATE_OBJECT(Material);
	Shader shader = Resources::FindShader("buildin/shaders/unlit_texture");
	material_->SetShader(shader);

	// TODO: set color for material instace, not shared.
	material_->SetVector4(Variables::mainColor, glm::vec4(1));
	material_->SetRenderState(Blend, SrcAlpha, OneMinusSrcAlpha);
}

FontInternal::~FontInternal() {
	Destroy();
}

bool FontInternal::Load(const std::string& path, int size) {
	Destroy();
	return Import("resources/" + path, size);
}

bool FontInternal::Require(const std::wstring& str) {
	bool status = true;
	for (int i = 0; i < str.length(); ++i) {
		if (!glyphs_.contains(str[i])) {
			Bitmap* bitmap = &glyphs_[str[i]]->bitmap;
			bitmap->id = str[i];

			status = GetBitmapBits(str[i], bitmap) && status;
			bitmaps_.push_back(bitmap);
			material_->SetTexture(Variables::mainTexture, nullptr);
		}
	}

	// TODO: rebuild timing.
	if (!material_->GetTexture(Variables::mainTexture)) {
		RebuildMaterial();
	}

	return status;
}

bool FontInternal::GetCharacterInfo(wchar_t wch, CharacterInfo* info) {
	Glyph* g;
	Atlas::CoordContainer::iterator pos = coords_.find(wch);
	if (pos == coords_.end() || !glyphs_.get(wch, g)) {
		Debug::LogError(std::to_string(wch) + " does not included");
		return false;
	}

	if (info != nullptr) {
		info->width = g->bitmap.width;
		info->height = g->bitmap.height;

		info->texCoord = pos->second;
	}

	return true;
}

bool FontInternal::Import(const std::string& path, int size) {
	int err = 0;
	if ((err = FT_Init_FreeType(&library_)) != 0) {
		Debug::LogError(String::Format("failed to load font %s (%d)", path.c_str(), err));
		return false;
	}

	if ((err = FT_New_Face(library_, path.c_str(), 0, &face_)) != 0) {
		Debug::LogError(String::Format("failed to create font face for %s (%d)", path.c_str(), err));
		return false;
	}

	FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
	FT_Set_Char_Size(face_, size << 6, size << 6, 96, 96);
	
	fname_ = Path::GetFileName(path);
	size_ = size;

	return true;
}

bool FontInternal::GetBitmapBits(wchar_t wch, Bitmap* answer) {
	int err = 0;

	if ((err = FT_Load_Glyph(face_, FT_Get_Char_Index(face_, wch), FT_LOAD_DEFAULT) != 0)) {
		Debug::LogError(String::Format("failed to load glyph for char %d (%d)", wch, err));
		return false;
	}

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

	std::vector<uchar>& data = answer->data;
	uint size = bitmap.width * bitmap.rows;
	data.resize(Math::Max(size, 1u));
	if (size != 0) {
		std::copy(bitmap.buffer, bitmap.buffer + size, &data[0]);
	}
	else {
		data[0] = 0;
	}

	answer->width = Math::Max(1u, bitmap.width);
	answer->height = Math::Max(1u, bitmap.rows);
	answer->format = ColorFormatLuminanceAlpha;

	return true;
}

void FontInternal::RebuildMaterial() {
	Atlas atlas;
	AtlasMaker::Make(atlas, bitmaps_, 4);

	coords_ = atlas.coords;

	Texture2D texture = CREATE_OBJECT(Texture2D);
	texture->Load(&atlas.data[0], ColorFormatLuminanceAlpha, atlas.width, atlas.height);

	material_->SetTexture(Variables::mainTexture, texture);
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
