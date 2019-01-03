#include "shadows.h"

#include <glm/gtc/matrix_transform.hpp>

#include "pipeline.h"
#include "resources.h"
#include "builtinproperties.h"
#include "sharedtexturemanager.h"
#include "internal/world/worldinternal.h"

Shadows::Shadows(RenderTexture dest) : shadowDepthTexture_(dest) {
	directionalLightShadowMaterial_ = new IMaterial();
	directionalLightShadowMaterial_->SetShader(Resources::FindShader("builtin/directional_light_depth"));
	directionalLightShadowMaterial_->SetRenderQueue((int)RenderQueue::Background - 200);
}

void Shadows::Resize(uint width, uint height) {
	shadowDepthTexture_->Resize(width, height);
}

void Shadows::Clear() {
	shadowDepthTexture_->Clear(Rect(0, 0, 1, 1), Color::black, 1);
}

RenderTexture Shadows::GetShadowTexture() {
	SUEDE_ASSERT(shadowDepthTexture_);
	return shadowDepthTexture_;
}

void Shadows::Update(Light light, Pipeline* pipeline) {
	glm::vec3 lightPosition = light->GetTransform()->GetPosition();
	glm::vec3 lightDirection = light->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;

	glm::mat4 projection = glm::ortho(-50.f, 50.f, -50.f, 50.f, near, far);
	glm::mat4 view = glm::lookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial_->SetMatrix4(BuiltinProperties::WorldToOrthographicLightMatrix, shadowDepthMatrix);

	uint nrenderables = pipeline->GetRenderableCount();
	Rect rect(0, 0, 1, 1);
	for (int i = 0; i < nrenderables; ++i) {
		Renderable& r = pipeline->GetRenderable(i);
		r.material = directionalLightShadowMaterial_;
		r.instance = 0;
	}

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
