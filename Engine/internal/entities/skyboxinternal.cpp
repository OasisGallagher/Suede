#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resources.h"
#include "variables.h"
#include "internal/memory/factory.h"
#include "internal/file/assetimporter.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/entities/skyboxinternal.h"

SkyboxInternal::SkyboxInternal() : EntityInternal(ObjectTypeSkybox) {
}

bool SkyboxInternal::Load(const std::string(&textures)[6]) {
	AssetImporter importer;
	importer.ImportTo(dsp_cast<Entity>(shared_from_this()), "buildin/models/box.obj");

	TextureCube texture = NewTextureCube();
	if (!texture->Load(textures)) {
		return false;
	}

	Shader shader = Resources::FindShader("buildin/shaders/skybox");

	// TODO: simple cube
	Renderer renderer = GetTransform()->FindChild("defaultobject")->GetEntity()->GetRenderer();
	Material material = renderer->GetMaterial(0);
	renderer->SetRenderQueue(RenderQueueBackground);
	
	material->SetShader(shader);
	material->SetTexture(Variables::mainTexture, texture);
	
	return true;
}
