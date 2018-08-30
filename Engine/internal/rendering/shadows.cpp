#include "shadows.h"

#include <glm/gtc/matrix_transform.hpp>

#include "pipeline.h"
#include "resources.h"
#include "variables.h"
#include "sharedtexturemanager.h"
#include "internal/world/worldinternal.h"

Shadows::Shadows() {
	uint w = Screen::instance()->GetWidth(), h = Screen::instance()->GetHeight();
	shadowDepthTexture_ = SharedTextureManager::instance()->GetShadowDepthTexture();

	directionalLightShadowMaterial_ = NewMaterial();
	directionalLightShadowMaterial_->SetShader(Resources::instance()->FindShader("builtin/directional_light_depth"));
	directionalLightShadowMaterial_->SetRenderQueue((int)RenderQueue::Background - 200);
}

void Shadows::Resize(uint width, uint height) {
	if (width != shadowDepthTexture_->GetWidth() || height != shadowDepthTexture_->GetHeight()) {
		shadowDepthTexture_->Resize(width, height);
	}
}

void Shadows::Clear() {
	shadowDepthTexture_->Clear(Rect(0, 0, 1, 1), glm::vec4(0, 0, 0, 1));
}

RenderTexture Shadows::GetShadowTexture() {
	return shadowDepthTexture_;
}

void Shadows::Update(DirectionalLight light, Pipeline* pipeline) {
	glm::vec3 lightPosition = light->GetTransform()->GetPosition();
	glm::vec3 lightDirection = light->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;

	glm::mat4 projection = glm::ortho(-50.f, 50.f, -50.f, 50.f, near, far);
	glm::mat4 view = glm::lookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial_->SetMatrix4(Variables::WorldToOrthographicLightMatrix, shadowDepthMatrix);

	uint nrenderables = pipeline->GetRenderableCount();
	Rect rect(0, 0, 1, 1);
	for (int i = 0; i < nrenderables; ++i) {
		Renderable& r = pipeline->GetRenderable(i);
		r.material = directionalLightShadowMaterial_;
		r.instance = 0;
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

	worldToShadowMatrix_ = bias * shadowDepthMatrix;
}

const glm::mat4& Shadows::GetWorldToShadowMatrix() {
	return worldToShadowMatrix_;
}
