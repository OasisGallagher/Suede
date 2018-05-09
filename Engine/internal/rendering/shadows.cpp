#include "shadows.h"
#include "pipeline.h"
#include "resources.h"
#include "variables.h"
#include "internal/base/framebuffer.h"
#include "internal/world/worldinternal.h"

Shadows* Shadows::Get() {
	static Shadows instance;
	return &instance;
}

Shadows::Shadows() {
	Screen::AddScreenSizeChangedListener(this);

	Initialize();
}

Shadows::~Shadows() {
	Screen::RemoveScreenSizeChangedListener(this);
}

void Shadows::OnScreenSizeChanged(uint width, uint height) {
	fbDepth->SetViewport(width, height);
	shadowDepthTexture->Resize(width, height);
}

void Shadows::AttachShadowTexture(Material material) {
	material->SetTexture(Variables::shadowDepthTexture, shadowDepthTexture);
}

void Shadows::Update(DirectionalLight light, Pipeline* pipeline, const std::vector<Entity>& entities) {
	fbDepth->Clear(FramebufferClearMaskDepth);

	FramebufferState state;
	fbDepth->SaveState(state);

	glm::vec3 lightPosition = light->GetTransform()->GetPosition();
	glm::vec3 lightDirection = light->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;

	glm::mat4 projection = glm::ortho(-50.f, 50.f, -50.f, 50.f, near, far);
	glm::mat4 view = glm::lookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial->SetMatrix4(Variables::worldToOrthographicLightMatrix, shadowDepthMatrix);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipeline->AddRenderable(entity->GetMesh(), directionalLightShadowMaterial, 0, state, entity->GetTransform()->GetLocalToWorldMatrix());
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

void Shadows::Initialize() {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();
	fbDepth = MEMORY_CREATE(Framebuffer);
	fbDepth->Create(w, h);

	shadowDepthTexture = NewRenderTexture();
	shadowDepthTexture->Load(RenderTextureFormatShadow, w, h);

	fbDepth->SetDepthTexture(shadowDepthTexture);

	directionalLightShadowMaterial = NewMaterial();
	directionalLightShadowMaterial->SetShader(Resources::FindShader("builtin/directional_light_depth"));
	directionalLightShadowMaterial->SetRenderQueue(RenderQueueBackground - 200);
}
