#include "shadows.h"
#include "pipeline.h"
#include "resources.h"
#include "variables.h"
#include "internal/base/framebuffer.h"
#include "internal/world/worldinternal.h"

static RenderTexture shadowDepthTexture;
static glm::mat4 worldToShadowMatrix;
static Material directionalLightShadowMaterial;

void Shadows::Initialize() {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();
	shadowDepthTexture = NewRenderTexture();
	shadowDepthTexture->Create(RenderTextureFormatShadow, w, h);

	directionalLightShadowMaterial = NewMaterial();
	directionalLightShadowMaterial->SetShader(Resources::FindShader("builtin/directional_light_depth"));
	directionalLightShadowMaterial->SetRenderQueue(RenderQueueBackground - 200);
}

void Shadows::Resize(uint width, uint height) {
	if (width != shadowDepthTexture->GetWidth() || height != shadowDepthTexture->GetHeight()) {
		shadowDepthTexture->Resize(width, height);
	}
}

void Shadows::AttachShadowTexture(Material material) {
	material->SetTexture(Variables::shadowDepthTexture, shadowDepthTexture);
}

void Shadows::Update(DirectionalLight light, Pipeline* pipeline, const std::vector<Entity>& entities) {
	shadowDepthTexture->Clear(Rect(0, 0, 1, 1), glm::vec4(0, 0, 0, 1));

	glm::vec3 lightPosition = light->GetTransform()->GetPosition();
	glm::vec3 lightDirection = light->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;

	glm::mat4 projection = glm::ortho(-50.f, 50.f, -50.f, 50.f, near, far);
	glm::mat4 view = glm::lookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial->SetMatrix4(Variables::worldToOrthographicLightMatrix, shadowDepthMatrix);

	uint nrenderables = pipeline->GetRenderableCount();
	Rect rect(0, 0, 1, 1);
	for (int i = 0; i < nrenderables; ++i) {
		Renderable& r = pipeline->GetRenderable(i);
		r.material = directionalLightShadowMaterial;
		r.target = shadowDepthTexture;
		r.normalizedRect = rect;
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
