#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "variables.h"
#include "internal/memory/factory.h"
#include "internal/file/assetimporter.h"
#include "internal/resources/resources.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/sprites/skyboxinternal.h"

SkyboxInternal::SkyboxInternal() : SpriteInternal(ObjectTypeSkybox) {
}

bool SkyboxInternal::Load(const std::string(&textures)[6]) {
	AssetImporter importer;
	importer.ImportTo(dsp_cast<Sprite>(shared_from_this()), "buildin/models/box.obj");

	TextureCube texture = CREATE_OBJECT(TextureCube);
	if (!texture->Load(textures)) {
		return false;
	}

	Shader shader = Resources::FindShader("buildin/shaders/skybox");

	// TODO: simple cube
	Renderer renderer = FindChild("defaultobject")->GetRenderer();
	Material material = renderer->GetMaterial(0);
	renderer->SetRenderQueue(RenderQueueBackground);
	material->SetRenderState(Cull, Front);
	material->SetRenderState(DepthTest, LessEqual);
	material->SetShader(shader);
	material->SetTexture(Variables::mainTexture, texture);
	
	return true;
}

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "tools/string.h"

class FontInternal {
public:
	virtual bool Load(const std::string& fname, unsigned height) {
		height_ = height;
		FT_Library library;
		int err = 0;
		if ((err = FT_Init_FreeType(&library)) != 0) {
			Debug::LogError(String::Format("failed to load font %s (%d)", fname.c_str(), err));
			return false;
		}

		FT_Face face;
		if ((err = FT_New_Face(library, fname.c_str(), 0, &face)) != 0) {
			Debug::LogError(String::Format("failed to create font face for %s (%d)", fname.c_str(), err));
			return false;
		}

		FT_Set_Char_Size(face, height << 6, height << 6, 96, 96);

		unsigned code = 0;
		if ((err = FT_Load_Glyph(face, FT_Get_Char_Index(face, code), FT_LOAD_DEFAULT)) != 0) {
			Debug::LogError(String::Format("failed to load glyph for char %d (%d)", code, err));
			return false;
		}

		FT_Glyph glyph;
		if ((err = FT_Get_Glyph(face->glyph, &glyph)) != 0) {
			Debug::LogError(String::Format("failed to get glyph for char %d (%d)", code, err));
			return false;
		}

		FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
		FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

		const FT_Bitmap& bitmap = bitmapGlyph->bitmap;

		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}

private:
	unsigned height_;
};
