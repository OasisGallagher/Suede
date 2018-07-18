#include <algorithm>

#include <ft2build.h>
#include <freetype/ftglyph.h>

#include "variables.h"
#include "resources.h"
#include "tools/math2.h"
#include "fontinternal.h"
#include "os/filesystem.h"

FontInternal::FontInternal() 
	: ObjectInternal(ObjectTypeFont) ,size_(10), face_(nullptr), library_(nullptr) {
	material_ = NewMaterial();
	material_->SetShader(Resources::get()->FindShader("builtin/unlit_texture"));
	material_->SetRenderQueue(RenderQueueTransparent);

	// default font color.
	material_->SetColor4(Variables::MainColor, glm::vec4(1));
	material_->SetTexture(Variables::MainTexture, NewTexture2D());
}

FontInternal::~FontInternal() {
	Destroy();
}

bool FontInternal::Load(const std::string& path, int size) {
	Destroy();
	return Import("resources/" + path, size);
}

bool FontInternal::Require(const std::wstring& str) {
	bool status = true, updateMaterial = false;
	for (int i = 0; i < str.length(); ++i) {
		if (!glyphs_.contains(str[i])) {
			TexelMap* texelMap = &glyphs_[str[i]]->texelMap;
			texelMap->id = str[i];

			status = GetBitmapBits(str[i], texelMap) && status;
			texelMaps_.push_back(texelMap);
			
			updateMaterial = true;
		}
	}

	if (updateMaterial) {
		RebuildMaterial();
	}

	return status;
}

Texture2D FontInternal::GetTexture() const {
	return suede_dynamic_cast<Texture2D>(material_->GetTexture(Variables::MainTexture));
}

std::string FontInternal::GetFamilyName() const {
	return face_->family_name;
}

std::string FontInternal::GetStyleName() const {
	return face_->style_name;
}

bool FontInternal::GetCharacterInfo(wchar_t wch, CharacterInfo* info) {
	Glyph* g;
	Atlas::CoordContainer::iterator pos = coords_.find(wch);
	if (pos == coords_.end() || !glyphs_.get(wch, g)) {
		Debug::LogError("%d does not included.", wch);
		return false;
	}

	if (info != nullptr) {
		info->width = g->texelMap.width;
		info->height = g->texelMap.height;

		info->texCoord = pos->second;
	}

	return true;
}

void FontInternal::AddMaterialRebuiltListener(FontMaterialRebuiltListener* listener) {
	if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
		listeners_.push_back(listener);
	}
}

void FontInternal::RemoveMaterialRebuiltListener(FontMaterialRebuiltListener* listener) {
	listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

bool FontInternal::Import(const std::string& path, int size) {
	int err = 0;
	if ((err = FT_Init_FreeType(&library_)) != 0) {
		Debug::LogError("failed to load font %s (%d)", path.c_str(), err);
		return false;
	}

	if ((err = FT_New_Face(library_, path.c_str(), 0, &face_)) != 0) {
		Debug::LogError("failed to create font face for %s (%d)", path.c_str(), err);
		return false;
	}

	FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
	FT_Set_Char_Size(face_, size << 6, size << 6, 96, 96);

	fname_ = FileSystem::GetFileName(path);
	size_ = size;

	return true;
}

bool FontInternal::GetBitmapBits(wchar_t wch, TexelMap* answer) {
	int err = 0;

	if ((err = FT_Load_Glyph(face_, FT_Get_Char_Index(face_, wch), FT_LOAD_DEFAULT) != 0)) {
		Debug::LogError("failed to load glyph for char %d (%d)", wch, err);
		return false;
	}

	FT_Glyph glyph;
	if ((err = FT_Get_Glyph(face_->glyph, &glyph)) != 0) {
		Debug::LogError("failed to get glyph for char %d (%d)", wch, err);
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
	answer->textureFormat = TextureFormatRgba;
	answer->colorStreamFormat = ColorStreamFormatLuminanceAlpha;
	answer->alignment = 4;

	return true;
}

void FontInternal::RebuildMaterial() {
	Atlas atlas;
	AtlasMaker::Make(atlas, texelMaps_, 4);

	coords_ = atlas.coords;

	Texture2D texture = suede_dynamic_cast<Texture2D>(material_->GetTexture(Variables::MainTexture));
	texture->Load(TextureFormatRgba, &atlas.data[0], ColorStreamFormatLuminanceAlpha, atlas.width, atlas.height, 4);

	for (uint i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnMaterialRebuilt();
	}
}

void FontInternal::Destroy() {
	if (face_ != nullptr) {
		FT_Done_Face(face_);
		face_ = nullptr;
	}

	if (library_ != nullptr) {
		FT_Done_FreeType(library_);
		library_ = nullptr;
	}

	fname_.clear();
}
