#include <glm/gtc/matrix_transform.hpp>

#include "light.h"
#include "camera.h"
#include "screen.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "internal/file/image.h"
#include "internal/base/gbuffer.h"
#include "internal/memory/factory.h"
#include "internal/base/framebuffer.h"
#include "internal/base/shaderinternal.h"
#include "internal/world/worldinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"
#include "internal/entities/camerainternal.h"

CameraInternal::CameraInternal() 
	: EntityInternal(ObjectTypeCamera)
	, fb1_(nullptr), fb2_(nullptr), gbuffer_(nullptr) {
	InitializeVariables();
	CreateFramebuffers();
	CreateDepthMaterial();
	CreateShadowMaterial();
	GL::ClearDepth(1);
}

CameraInternal::~CameraInternal() {
	MEMORY_RELEASE(fb1_);
	MEMORY_RELEASE(fb2_);
	MEMORY_RELEASE(gbuffer_);
}

void CameraInternal::SetClearColor(const glm::vec3 & value) {
	Framebuffer0::Get()->SetClearColor(value);
}

glm::vec3 CameraInternal::GetClearColor() {
	return Framebuffer0::Get()->GetClearColor();
}

void CameraInternal::SetAspect(float value) {
	if (!Math::Approximately(aspect_, value)) {
		aspect_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetNearClipPlane(float value) {
	if (!Math::Approximately(near_, value)) {
		near_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetFarClipPlane(float value) {
	if (!Math::Approximately(far_, value)) {
		far_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetFieldOfView(float value) {
	if (!Math::Approximately(near_, value)) {
		near_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetRenderTexture(RenderTexture value) {
	fb1_->SetRenderTexture(FramebufferAttachment0, value);
}

void CameraInternal::Update() {
	int w = Screen::GetWidth();
	int h = Screen::GetHeight();

	Framebuffer0* fb0 = Framebuffer0::Get();
	if (w != fb0->GetViewportWidth() || h != fb0->GetViewportHeight()) {
		fb0->SetViewport(w, h);
		OnContextSizeChanged(w, h);
	}

	UpdateSkybox();
}

void CameraInternal::Render() {
	std::vector<Entity> entities;
	GetRenderableEntities(entities);

	if (renderPath_ == RenderPathForward) {
		if ((depthTextureMode_ & DepthTextureModeDepth) != 0) {
			ForwardDepthPass(entities);
		}
	}

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	ShadowDepthPass(entities, forwardBase);
	
	// TODO: switch framebuffer is expensive.
	fb1_->SetDepthTexture(nullptr);

	FramebufferBase* active = GetActiveFramebuffer();
	active->BindWrite();

	if (renderPath_ == RenderPathForward) {
		ForwardRendering(entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(entities, forwardBase, forwardAdd);
	}

	OnPostRender();

	active->Unbind();

	if (!imageEffects_.empty()) {
		OnImageEffects();
	}

	pass_ = RenderPassNone;
}

void CameraInternal::ForwardRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (forwardBase) {
		RenderForwardBase(entities, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(entities, forwardAdd);
	}
}

void CameraInternal::DeferredRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (gbuffer_ == nullptr) {
		InitializeDeferredRender();
	}

	RenderDeferredGeometryPass(entities);
}

void CameraInternal::InitializeDeferredRender() {
	gbuffer_ = MEMORY_CREATE(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	Shader shader = NewShader();
	shader->Load("buildin/shaders/gbuffer");
	deferredMaterial_->SetShader(shader);
}

void CameraInternal::RenderDeferredGeometryPass(const std::vector<Entity>& entities) {
	pass_ = RenderPassDeferredGeometryPass;
	gbuffer_->Bind(GBuffer::GeometryPass);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		//if (entity->GetRenderer()->GetRenderQueue() < RenderQueueGeometry) {
		//	continue;
		//}

		glm::mat4 localToClipSpaceMatrix = projection_ * GetWorldToLocalMatrix() * entity->GetLocalToWorldMatrix();
		deferredMaterial_->SetMatrix4(Variables::localToClipSpaceMatrix, localToClipSpaceMatrix);
		deferredMaterial_->SetMatrix4(Variables::localToWorldSpaceMatrix, entity->GetLocalToWorldMatrix());

		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
		deferredMaterial_->SetTexture(Variables::mainTexture, mainTexture);

		// TODO: mesh renderer.
		Resources::GetMeshRenderer()->SetMaterial(0, deferredMaterial_);
		Resources::GetMeshRenderer()->RenderEntity(entity);
	}

	gbuffer_->Unbind();
}

glm::vec3 CameraInternal::WorldToScreenPoint(const glm::vec3& position) {
	glm::ivec4 viewport;
	GL::GetIntegerv(GL_VIEWPORT, (GLint*)&viewport);
	return glm::project(position, GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

glm::vec3 CameraInternal::ScreenToWorldPoint(const glm::vec3& position) {
	glm::ivec4 viewport;
	GL::GetIntegerv(GL_VIEWPORT, (GLint*)&viewport);
	return glm::unProject(position, GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

Texture2D CameraInternal::Capture() {
	std::vector<uchar> data;
	Framebuffer0::Get()->ReadBuffer(data);

	Texture2D texture = NewTexture2D();
	texture->Load(TextureFormatRgb, &data[0], ColorStreamFormatRgb, Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	return texture;
}

void CameraInternal::InitializeVariables() {
	depth_ = 0; 
	aspect_ = 1.3f;
	near_ = 1.f;
	far_ = 1000.f;
	fieldOfView_ = Math::Pi() / 3.f;
	projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);

	clearType_ = ClearTypeColor;
	pass_ = RenderPassNone;
	renderPath_ = RenderPathForward;
}

void CameraInternal::CreateFramebuffers() {
	int w = Screen::GetWidth();
	int h = Screen::GetHeight();

	fb1_ = MEMORY_CREATE(Framebuffer);
	fb1_->Create(w, h);

	depthTexture_ = NewRenderTexture();
	depthTexture_->Load(RenderTextureFormatDepth, w, h);

	shadowTexture_ = NewRenderTexture();
	shadowTexture_->Load(RenderTextureFormatShadow, w, h);
}

void CameraInternal::CreateDepthMaterial() {
	Shader shader = Resources::FindShader("buildin/shaders/depth");

	depthMaterial_ = NewMaterial();
	depthMaterial_->SetShader(shader);
}

void CameraInternal::CreateShadowMaterial() {
	Shader shader = Resources::FindShader("buildin/shaders/directional_light_depth");
	directionalLightShadowMaterial_ = NewMaterial();
	directionalLightShadowMaterial_->SetShader(shader);
}

void CameraInternal::UpdateSkybox() {
	if (clearType_ != ClearTypeSkybox) { return; }
	Skybox skybox = skybox_;
	if (!skybox) {
		skybox = WorldInstance()->GetEnvironment()->GetSkybox();
	}

	if (!skybox) {
		Debug::LogError("skybox does not exist.");
		return;
	}

	skybox->SetPosition(GetPosition());
}

void CameraInternal::OnContextSizeChanged(int w, int h) {
	fb1_->SetViewport(w, h);

	if (fb2_ != nullptr) {
		fb2_->SetViewport(w, h);
	}

	float aspect = (float)w / h;
	if (!Math::Approximately(aspect, GetAspect())) {
		SetAspect(aspect);
	}
}

FramebufferBase* CameraInternal::GetActiveFramebuffer() {
	FramebufferBase* active = nullptr;

	if (!imageEffects_.empty()) {
		SetUpFramebuffer1();
	}

	if (fb1_->GetRenderTexture(FramebufferAttachment0)) {
		active = fb1_;
	}
	else {
		active = Framebuffer0::Get();
	}

	return active;
}

void CameraInternal::SetUpFramebuffer1() {
	if (!fb1_->GetRenderTexture(FramebufferAttachment0)) {
		if (!renderTexture_) {
			renderTexture_ = NewRenderTexture();
			renderTexture_->Load(RenderTextureFormatRgba, Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());
		}

		fb1_->SetRenderTexture(FramebufferAttachment0, renderTexture_);
	}
}

void CameraInternal::CreateFramebuffer2() {
	if (fb2_ == nullptr) {
		fb2_ = MEMORY_CREATE(Framebuffer);
		fb2_->Create(fb1_->GetViewportWidth(), fb1_->GetViewportHeight());

		renderTexture2_ = NewRenderTexture();
		renderTexture2_->Load(RenderTextureFormatRgba, fb2_->GetViewportWidth(), fb2_->GetViewportHeight());
		fb2_->SetRenderTexture(FramebufferAttachment0, renderTexture2_);
	}
}

void CameraInternal::SetForwardBaseLightParameter(const std::vector<Entity>& entities, Light light) {
	for (int i = 0; i < entities.size(); ++i) {
		Renderer renderer = entities[i]->GetRenderer();
		int materialCount = renderer->GetMaterialCount();
		for (int i = 0; i < materialCount; ++i) {
			Material material = renderer->GetMaterial(i);

			material->SetVector3(Variables::cameraPosition, GetPosition());
			material->SetColor3(Variables::ambientLightColor, WorldInstance()->GetEnvironment()->GetAmbientColor());
			material->SetColor3(Variables::lightColor, light->GetColor());
			material->SetVector3(Variables::lightPosition, light->GetPosition());
			material->SetVector3(Variables::lightDirection, light->GetRotation() * glm::vec3(0, 0, -1));
		}
	}
}

void CameraInternal::RenderForwardBase(const std::vector<Entity>& entities, Light light) {
	SetForwardBaseLightParameter(entities, light);

	int from = 0;
	from = ForwardBackgroundPass(entities, from);
	from = ForwardOpaquePass(entities, from);
	from = ForwardTransparentPass(entities, from);
}

void CameraInternal::ShadowDepthPass(const std::vector<Entity>& entities, Light light) {
	pass_ = RenderPassShadowDepth;
	if (light->GetType() != ObjectTypeDirectionalLight) {
		Debug::LogError("invalid light type");
		return;
	}

	fb1_->SetDepthTexture(shadowTexture_);
	fb1_->BindWriteAttachment(FramebufferAttachmentNone);

	glm::vec3 lightPosition = light->GetRotation() * glm::vec3(0, 0, 1);
	glm::mat4 projection = glm::ortho(-100.f, 100.f, -100.f, 100.f, -100.f, 100.f);
	glm::mat4 view = glm::lookAt(lightPosition * 10.f, glm::vec3(0), light->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		if (entity->GetRenderer()->GetRenderQueue() < RenderQueueGeometry) {
			continue;
		}

		directionalLightShadowMaterial_->SetMatrix4(Variables::localToOrthographicLightSpaceMatrix, shadowDepthMatrix * entity->GetLocalToWorldMatrix());
		Resources::GetMeshRenderer()->SetMaterial(0, directionalLightShadowMaterial_);
		Resources::GetMeshRenderer()->RenderEntity(entity);
	}

	glm::mat4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	viewToShadowSpaceMatrix_ = bias * shadowDepthMatrix;
	fb1_->Unbind();
}

void CameraInternal::RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights) {
}

int CameraInternal::ForwardBackgroundPass(const std::vector<Entity>& entities, int from) {
	pass_ = RenderPassForwardBackground;
	return 0;
}

void CameraInternal::ForwardDepthPass(const std::vector<Entity>& entities) {
	pass_ = RenderPassForwardDepth;

	fb1_->SetDepthTexture(depthTexture_);

	fb1_->BindWriteAttachment(FramebufferAttachmentNone);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		if (entity->GetRenderer()->GetRenderQueue() < RenderQueueGeometry) {
			continue;
		}

		glm::mat4 localToClipSpaceMatrix = projection_ * GetWorldToLocalMatrix() * entity->GetLocalToWorldMatrix();
		depthMaterial_->SetMatrix4(Variables::localToClipSpaceMatrix, localToClipSpaceMatrix);

		// TODO: mesh renderer.
		Resources::GetMeshRenderer()->SetMaterial(0, depthMaterial_);
		Resources::GetMeshRenderer()->RenderEntity(entity);
	}

	fb1_->Unbind();
}

int CameraInternal::ForwardOpaquePass(const std::vector<Entity>& entities, int from) {
	pass_ = RenderPassForwardOpaque;
	// TODO: sort.
	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		RenderEntity(entity, entity->GetRenderer());
	}

	return 0;
}

int CameraInternal::ForwardTransparentPass(const std::vector<Entity>& entities, int from) {
	pass_ = RenderPassForwardTransparent;
	// TODO: sort.
	return 0;
}

void CameraInternal::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Entity> lights;
	if (!WorldInstance()->GetEntities(ObjectTypeLights, lights)) {
		return;
	}

	forwardBase = dsp_cast<Light>(lights.front());
	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(dsp_cast<Light>(lights[i]));
	}
}

void CameraInternal::OnPostRender() {
}

void CameraInternal::OnImageEffects() {
	CreateFramebuffer2();

	FramebufferBase* framebuffers[] = { fb1_, fb2_ };
	RenderTexture textures[] = { fb1_->GetRenderTexture(FramebufferAttachment0), renderTexture2_ };

	int index = 1;
	for (int i = 0; i < imageEffects_.size(); ++i) {
		FramebufferBase* active = framebuffers[index];
		
		if (i + 1 == imageEffects_.size()) {
			active = Framebuffer0::Get();
			textures[index] = nullptr;
		}

		active->BindWrite();
		imageEffects_[i]->OnRenderImage(textures[1 - index], textures[index]);

		active->Unbind();
		index = 1 - index;
	}
}

bool CameraInternal::IsRenderable(Entity entity) {
	return entity->GetActive() && entity->GetRenderer()
		&& entity->GetMesh() && entity->GetMesh()->GetSubMeshCount() > 0;
}

void CameraInternal::GetRenderableEntities(std::vector<Entity>& entities) {
	WorldInstance()->GetEntities(ObjectTypeEntity, entities);
	SortRenderableEntities(entities);
}

void CameraInternal::SortRenderableEntities(std::vector<Entity>& entities) {
	int p = 0;
	for (int i = 0; i < entities.size(); ++i) {
		Entity key = entities[i];
		if (!IsRenderable(key)) {
			continue;
		}

		int j = p - 1;

		for (; j >= 0 && entities[j]->GetRenderer()->GetRenderQueue() > key->GetRenderer()->GetRenderQueue(); --j) {
			entities[j + 1] = entities[j];
		}

		entities[j + 1] = key;
		++p;
	}

	entities.erase(entities.begin() + p, entities.end());
}

void CameraInternal::RenderEntity(Entity entity, Renderer renderer) {
	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		UpdateMaterial(entity, renderer->GetMaterial(i));
	}

	renderer->RenderEntity(entity);
}

void CameraInternal::UpdateMaterial(Entity entity, Material material) {
	glm::mat4 localToWorldMatrix = entity->GetLocalToWorldMatrix();
	glm::mat4 worldToCameraSpaceMatrix = GetWorldToLocalMatrix();
	glm::mat4 worldToClipSpaceMatrix = projection_ * worldToCameraSpaceMatrix;
	glm::mat4 localToClipSpaceMatrix = worldToClipSpaceMatrix * localToWorldMatrix;

	// TODO: uniform buffers for common matrices.
	material->SetMatrix4(Variables::worldToClipSpaceMatrix, worldToClipSpaceMatrix);
	material->SetMatrix4(Variables::worldToCameraSpaceMatrix, worldToCameraSpaceMatrix);
	material->SetMatrix4(Variables::localToClipSpaceMatrix, localToClipSpaceMatrix);

	if (pass_ >= RenderPassForwardOpaque) {
		material->SetMatrix4(Variables::localToShadowSpaceMatrix, viewToShadowSpaceMatrix_ * localToWorldMatrix);
		material->SetTexture(Variables::shadowDepthTexture, shadowTexture_);
	}
}
