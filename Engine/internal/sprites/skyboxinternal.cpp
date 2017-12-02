#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resources.h"
#include "variables.h"
#include "internal/memory/factory.h"
#include "internal/file/assetimporter.h"
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
