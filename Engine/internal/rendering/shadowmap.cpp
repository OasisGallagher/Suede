#include "shadowmap.h"

#include "pipeline.h"
#include "resources.h"
#include "builtinproperties.h"
#include "internal/world/worldinternal.h"

ShadowMap::ShadowMap(RenderTexture* target) : shadowDepthTexture_(target){
	directionalLightShadowMaterial_ = new Material();
	directionalLightShadowMaterial_->SetShader(Resources::FindShader("builtin/directional_light_depth"));
	directionalLightShadowMaterial_->SetRenderQueue((int)RenderQueue::Background - 200);
}

void ShadowMap::Resize(uint width, uint height) {
	if (width != shadowDepthTexture_->GetWidth() || height != shadowDepthTexture_->GetHeight()) {
		shadowDepthTexture_->Resize(width, height);
	}
}

void ShadowMap::Clear() {
	shadowDepthTexture_->Clear(Rect(0, 0, 1, 1), Color::black, 1);
}

void ShadowMap::Update(Light* light) {
	Vector3 lightPosition = light->GetTransform()->GetPosition();
	Vector3 lightDirection = light->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;

	Matrix4 projection = Matrix4::Ortho(-50.f, 50.f, -50.f, 50.f, near, far);
	Matrix4 view = Matrix4::LookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	Matrix4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial_->SetMatrix4(BuiltinProperties::WorldToOrthographicLightMatrix, shadowDepthMatrix);

	Matrix4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	worldToShadowMatrix_ = bias * shadowDepthMatrix;
}

const Matrix4& ShadowMap::GetWorldToShadowMatrix() {
	return worldToShadowMatrix_;
}
