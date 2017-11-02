#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "internal/memory/factory.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/sprites/skyboxinternal.h"

SkyboxInternal::SkyboxInternal() : SpriteInternal(ObjectTypeSkybox) {
}

bool SkyboxInternal::Load(const std::string(&textures)[6]) {
	if (!LoadModel("buildin/models/box.obj")) {
		return false;
	}

	TextureCube texture = CREATE_OBJECT(TextureCube);
	if (!texture->Load(textures)) {
		return false;
	}

	Shader shader = CREATE_OBJECT(Shader);
	if (!shader->Load("buildin/shaders/skybox")) {
		return false;
	}

	MaterialTextures& materialTextures = GetSurface()->GetMesh(0)->GetMaterialTextures();
	materialTextures.albedo = texture;

	Renderer renderer = GetRenderer();
	renderer->SetRenderQueue(RenderQueueBackground);
	renderer->SetRenderState(Cull, Front);
	renderer->SetRenderState(DepthTest, LessEqual);
	renderer->GetMaterial(0)->SetShader(shader);

	return true;
}
