#include "shadows.h"
#include "pipeline.h"
#include "resources.h"
#include "variables.h"
#include "internal/base/framebuffer.h"
#include "internal/world/worldinternal.h"

static RenderTexture rtShadow;
static glm::mat4 worldToShadowMatrix;
static Material directionalLightShadowMaterial;

void Shadows::Initialize() {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();
	rtShadow = NewRenderTexture();
	rtShadow->Create(RenderTextureFormatShadow, w, h);

	directionalLightShadowMaterial = NewMaterial();
	directionalLightShadowMaterial->SetShader(Resources::FindShader("builtin/directional_light_depth"));
	directionalLightShadowMaterial->SetRenderQueue(RenderQueueBackground - 200);
}

void Shadows::Resize(uint width, uint height) {
	if (width != rtShadow->GetWidth() || height != rtShadow->GetHeight()) {
		rtShadow->Resize(width, height);
	}
}

void Shadows::AttachShadowTexture(Material material) {
	material->SetTexture(Variables::shadowDepthTexture, rtShadow);
}

void Shadows::Update(DirectionalLight light, Pipeline* pipeline, const std::vector<Entity>& entities) {
	rtShadow->Clear(glm::vec4(0, 0, 0, 1));

	glm::vec3 lightPosition = light->GetTransform()->GetPosition();
	glm::vec3 lightDirection = light->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;

	glm::mat4 projection = glm::ortho(-50.f, 50.f, -50.f, 50.f, near, far);
	glm::mat4 view = glm::lookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial->SetMatrix4(Variables::worldToOrthographicLightMatrix, shadowDepthMatrix);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipeline->AddRenderable(entity->GetMesh(), directionalLightShadowMaterial, 0, rtShadow, glm::vec4(0, 0, 1, 1), entity->GetTransform()->GetLocalToWorldMatrix());
	}

	/*
	glm::vec3 center(0, 0, -(near + far) / 2);
	glm::vec3 size(100, 100, far - near);
	center = glm::vec3(directionalLight->GetTransform()->GetLocalToWorldMatrix() * glm::vec4(center, 1));
	Gizmos::DrawCuboid(center, size);
	*/

	glm::mat4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	worldToShadowMatrix = bias * shadowDepthMatrix;
}

const glm::mat4& Shadows::GetWorldToShadowMatrix() {
	return worldToShadowMatrix;
}
