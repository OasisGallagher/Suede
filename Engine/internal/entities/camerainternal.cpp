#include <glm/gtx/transform.hpp>

#include "time2.h"
#include "gizmos.h"
#include "screen.h"
#include "resources.h"
#include "variables.h"
#include "imageeffect.h"
#include "gizmospainter.h"
#include "debug/profiler.h"
#include "geometryutility.h"
#include "internal/base/gbuffer.h"
#include "internal/rendering/pipeline.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/camerainternal.h"
#include "internal/rendering/uniformbuffermanager.h"

CameraInternal::CameraInternal()
	: EntityInternal(ObjectTypeCamera), depthTextureMode_(DepthTextureModeNone)
	, fb1_(nullptr), fb2_(nullptr), fbDepth_(nullptr), gbuffer_(nullptr) {
	pipeline_ = MEMORY_CREATE(Pipeline);

	forward_pass = Profiler::CreateSample();
	push_renderables = Profiler::CreateSample();
	get_renderable_entities = Profiler::CreateSample();

	InitializeVariables();
	CreateFramebuffers();

	CreateAuxMaterial(depthMaterial_, "builtin/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(decalMaterial_, "builtin/decal", RenderQueueOverlay - 500);
	CreateAuxMaterial(directionalLightShadowMaterial_, "builtin/directional_light_depth", RenderQueueBackground - 200);
}

CameraInternal::~CameraInternal() {
	MEMORY_RELEASE(fb1_);
	MEMORY_RELEASE(fb2_);
	MEMORY_RELEASE(fbDepth_);

	MEMORY_RELEASE(gbuffer_);
	MEMORY_RELEASE(pipeline_);

	Profiler::ReleaseSample(forward_pass);
	Profiler::ReleaseSample(push_renderables);
	Profiler::ReleaseSample(get_renderable_entities);
}

void CameraInternal::SetClearColor(const glm::vec3 & value) {
	Framebuffer0::Get()->SetClearColor(value);
}

glm::vec3 CameraInternal::GetClearColor() {
	return Framebuffer0::Get()->GetClearColor();
}

void CameraInternal::SetRenderTexture(RenderTexture value) {
	fb1_->SetRenderTexture(FramebufferAttachment0, value);
}

void CameraInternal::Update() {
	UpdateViewportSize();

	// Stub: main thread only.
	ClearFramebuffers();
	UpdateTimeUniformBuffer();
}

void CameraInternal::Render() {
	std::vector<Entity> entities;
	glm::mat4 worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	get_renderable_entities->Restart();
	WorldInstance()->GetVisibleEntities(entities, worldToClipMatrix);
	get_renderable_entities->Stop();

	Debug::Output("[CameraInternal::Render::get_renderable_entities]\t%.2f\n", get_renderable_entities->GetElapsedSeconds());

	if (renderPath_ == RenderPathForward) {
		if ((depthTextureMode_ & DepthTextureModeDepth) != 0) {
			ForwardDepthPass(entities);
		}
	}
	
	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	if (forwardBase) {
		ShadowDepthPass(entities, forwardBase);
	}

	UpdateTransformsUniformBuffer();

	FramebufferState state;
	GetActiveFramebuffer()->SaveState(state);

	if (renderPath_ == RenderPathForward) {
		ForwardRendering(state, entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(state, entities, forwardBase, forwardAdd);
	}

	//  Stub: main thread only.
	pipeline_->Flush(worldToClipMatrix);

	OnPostRender();

	// Stub: main thread only.
	if (!imageEffects_.empty()) {
		OnImageEffects();
	}

	OnDrawGizmos();
}

void CameraInternal::OnProjectionMatrixChanged() {
	GeometryUtility::CalculateFrustumPlanes(planes_, GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
}

void CameraInternal::ClearFramebuffers() {
	fb1_->Clear(FramebufferClearMaskColorDepth);

	if (fb2_ != nullptr) {
		fb2_->Clear(FramebufferClearMaskColorDepth);
	}
}

void CameraInternal::UpdateViewportSize() {
	int w = Screen::GetWidth();
	int h = Screen::GetHeight();

	Framebuffer0* fb0 = Framebuffer0::Get();
	if (w != fb0->GetViewportWidth() || h != fb0->GetViewportHeight()) {
		fb0->SetViewport(w, h);
		OnViewportSizeChanged(w, h);
	}
}

void CameraInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::GetRealTimeSinceStartup();
	p.time.y = Time::GetDeltaTime();
	UniformBufferManager::UpdateSharedBuffer(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
}
/*
for (int i = 0; i < 4; ++i) {
	// use either :
	//  - Always the same samples.
	//    Gives a fixed pattern in the shadow, but no noise
	int index = i;
	//  - A random sample, based on the pixel's screen location. 
	//    No banding, but the shadow moves with the camera, which looks weird.
	// int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
	//  - A random sample, based on the pixel's position in world space.
	//    The position is rounded to the millimeter to avoid too much aliasing
	// int index = int(16.0 * random(floor(worldPos.xyz * 1000.0), i)) % 16;

	// being fully in the shadow will eat up 4*0.2 = 0.8
	// 0.2 potentially remain, which is quite dark.
	visibility -= 0.2 * (
		1 - texture(
			c_shadowDepthTexture, vec3(
				c_shadowCoord.xy + poissonDisk[index] / 700.0, (c_shadowCoord.z - bias) / c_shadowCoord.w
			)
		)
	);
}
*/
void CameraInternal::UpdateTransformsUniformBuffer() {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	p.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	p.cameraToClipMatrix = GetProjectionMatrix();
	p.worldToShadowMatrix = worldToShadowMatrix_;

	p.cameraPosition = glm::vec4(GetTransform()->GetPosition(), 1);
	UniformBufferManager::UpdateSharedBuffer(SharedTransformsUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void CameraInternal::ForwardRendering(const FramebufferState& state, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (clearType_ == ClearTypeSkybox) {
		RenderSkybox(state);
	}
	
	if (forwardBase) {
		RenderForwardBase(state, entities, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(entities, forwardAdd);
	}
	
	RenderDecals(state);
}

void CameraInternal::DeferredRendering(const FramebufferState& state, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (gbuffer_ == nullptr) {
		InitializeDeferredRender();
	}

	RenderDeferredGeometryPass(state, entities);
}

void CameraInternal::InitializeDeferredRender() {
	gbuffer_ = MEMORY_CREATE(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::FindShader("builtin/gbuffer"));
}

void CameraInternal::AddToPipeline(const FramebufferState& state, Mesh mesh, Material material, const glm::mat4& localToWorldMatrix) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		pipeline_->AddRenderable(mesh, i, material, 0, state, localToWorldMatrix);
	}
}

void CameraInternal::RenderDeferredGeometryPass(const FramebufferState& state, const std::vector<Entity>& entities) {
	gbuffer_->Bind(GBuffer::GeometryPass);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];

		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
		material->SetTexture(Variables::mainTexture, mainTexture);
		AddToPipeline(state, entity->GetMesh(), deferredMaterial_, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	gbuffer_->Unbind();
}

glm::vec3 CameraInternal::WorldToScreenPoint(const glm::vec3& position) {
	glm::ivec4 viewport;
	GL::GetIntegerv(GL_VIEWPORT, (GLint*)&viewport);
	return glm::project(position, GetTransform()->GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

glm::vec3 CameraInternal::ScreenToWorldPoint(const glm::vec3& position) {
	glm::ivec4 viewport;
	GL::GetIntegerv(GL_VIEWPORT, (GLint*)&viewport);
	return glm::unProject(position, GetTransform()->GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
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
	clearType_ = ClearTypeColor;
	renderPath_ = RenderPathForward;
}

void CameraInternal::CreateFramebuffers() {
	int w = Screen::GetWidth();
	int h = Screen::GetHeight();

	fb1_ = MEMORY_CREATE(Framebuffer);
	fb1_->Create(w, h);
	fb1_->CreateDepthRenderbuffer();

	fbDepth_ = MEMORY_CREATE(Framebuffer);
	fbDepth_->Create(w, h);

	depthTexture_ = NewRenderTexture();
	depthTexture_->Load(RenderTextureFormatDepth, w, h);

	shadowTexture_ = NewRenderTexture();
	shadowTexture_->Load(RenderTextureFormatDepth, w, h);
}

void CameraInternal::CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue) {
	Shader shader = Resources::FindShader(shaderPath);
	material = NewMaterial();
	material->SetShader(shader);
	material->SetRenderQueue(renderQueue);
}

void CameraInternal::RenderSkybox(const FramebufferState& state) {
	Material skybox = WorldInstance()->GetEnvironment()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = GetTransform()->GetWorldToLocalMatrix();
		matrix[3] = glm::vec4(0, 0, 0, 1);
		AddToPipeline(state, Resources::GetPrimitive(PrimitiveTypeCube), skybox, matrix);
	}
}

void CameraInternal::OnViewportSizeChanged(int w, int h) {
	fb1_->SetViewport(w, h);
	fbDepth_->SetViewport(w, h);

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
		fb2_->CreateDepthRenderbuffer();
	}
}

void CameraInternal::UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light) {
	static SharedLightUniformBuffer p;
	p.ambientLightColor = glm::vec4(WorldInstance()->GetEnvironment()->GetAmbientColor(), 1);
	p.lightColor = glm::vec4(light->GetColor(), 1);
	p.lightPosition = glm::vec4(light->GetTransform()->GetPosition(), 1);
	p.lightDirection = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	UniformBufferManager::UpdateSharedBuffer(SharedLightUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void CameraInternal::RenderForwardBase(const FramebufferState& state, const std::vector<Entity>& entities, Light light) {
	// Stub: GL.
	UpdateForwardBaseLightUniformBuffer(entities, light);

	forward_pass->Restart();
	ForwardPass(state, entities);
	forward_pass->Stop();
	Debug::Output("[CameraInternal::RenderForwardBase::forward_pass]\t%.2f\n", forward_pass->GetElapsedSeconds());
}

void CameraInternal::ShadowDepthPass(const std::vector<Entity>& entities, Light light) {
	if (light->GetType() != ObjectTypeDirectionalLight) {
		Debug::LogError("invalid light type");
		return;
	}

	fbDepth_->SetDepthTexture(shadowTexture_);
	fbDepth_->Clear(FramebufferClearMaskDepth);
	
	FramebufferState state;
	fbDepth_->SaveState(state);

	DirectionalLight directionalLight = suede_dynamic_cast<DirectionalLight>(light);

	glm::vec3 lightPosition = directionalLight->GetTransform()->GetPosition();
	glm::vec3 lightDirection = directionalLight->GetTransform()->GetForward();
	float near = 1.f, far = 90.f;
	glm::mat4 projection = glm::ortho(-20.f, 20.f, -20.f, 20.f, near, far);
	glm::mat4 view = glm::lookAt(lightPosition, lightPosition + lightDirection, light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;
	directionalLightShadowMaterial_->SetMatrix4(Variables::worldToOrthographicLightMatrix, shadowDepthMatrix);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		AddToPipeline(state, entity->GetMesh(), directionalLightShadowMaterial_, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	glm::vec3 center(0, 0, (near + far) / 2);
	glm::vec3 size(40, 40, far - near);
	center = glm::vec3(directionalLight->GetTransform()->GetLocalToWorldMatrix() * glm::vec4(center, 1));
	Gizmos::DrawCuboid(center, size);

	glm::mat4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	worldToShadowMatrix_ = bias * shadowDepthMatrix;
}

void CameraInternal::RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights) {
}

void CameraInternal::ForwardDepthPass(const std::vector<Entity>& entities) {
	fbDepth_->SetDepthTexture(depthTexture_);
	fbDepth_->Clear(FramebufferClearMaskDepth);

	FramebufferState state;
	fbDepth_->SaveState(state);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		AddToPipeline(state, entity->GetMesh(), depthMaterial_, entity->GetTransform()->GetLocalToWorldMatrix());
	}
}

void CameraInternal::ForwardPass(const FramebufferState& state, const std::vector<Entity>& entities) {
	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		RenderEntity(state, entity, entity->GetRenderer());
	}

	Debug::Output("[CameraInternal::ForwardPass::push_renderables]\t%.2f\n", push_renderables->GetElapsedSeconds());
	push_renderables->Clear();
}

void CameraInternal::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Entity> lights;
	if (!WorldInstance()->GetEntities(ObjectTypeLights, lights, nullptr)) {
		return;
	}

	forwardBase = suede_dynamic_cast<Light>(lights.front());
	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(suede_dynamic_cast<Light>(lights[i]));
	}
}

void CameraInternal::OnPostRender() {
	
}

void CameraInternal::RenderDecals(const FramebufferState& state) {
	std::vector<Decal*> decals;
	WorldInstance()->GetDecals(decals);

	for (int i = 0; i < decals.size(); ++i) {
		Decal* d = decals[i];
		glm::mat4 biasMatrix = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.5f)), glm::vec3(0.5f));
		Material decalMaterial = suede_dynamic_cast<Material>(decalMaterial_->Clone());

		decalMaterial->SetMatrix4(Variables::decalMatrix, biasMatrix * d->matrix);
		decalMaterial->SetTexture(Variables::mainTexture, d->texture);

		Mesh mesh = NewMesh();

		MeshAttribute attribute;
		attribute.topology = d->topology;
		attribute.indexes = d->indexes;
		attribute.positions = d->positions;

		mesh->SetAttribute(attribute);

		SubMesh subMesh = NewSubMesh();
		TriangleBias bias{ d->indexes.size() };
		subMesh->SetTriangleBias(bias);

		mesh->AddSubMesh(subMesh);

		AddToPipeline(state, mesh, decalMaterial, glm::mat4(1));
	}
}

void CameraInternal::OnDrawGizmos() {
	for (int i = 0; i < gizmosPainters_.size(); ++i) {
		gizmosPainters_[i]->OnDrawGizmos();
	}

	Gizmos::Flush();
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

		active->BindWrite(FramebufferClearMaskNone);
		imageEffects_[i]->OnRenderImage(shadowTexture_/*textures[1 - index]*/, textures[index]);
		active->Unbind();

		index = 1 - index;
	}
}

void CameraInternal::RenderEntity(const FramebufferState& state, Entity entity, Renderer renderer) {
	push_renderables->Start();
	
	int subMeshCount = entity->GetMesh()->GetSubMeshCount();
	int materialCount = renderer->GetMaterialCount();

	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	// TODO: update shadowTexture property only once.
	for (uint i = 0; i < renderer->GetMaterialCount(); ++i) {
		renderer->GetMaterial(i)->SetTexture(Variables::shadowDepthTexture, shadowTexture_);
	}

	renderer->UpdateMaterialProperties();

	for (int i = 0; i < subMeshCount; ++i) {
		Material material = renderer->GetMaterial(i);
		int pass = material->GetPass();
		if (pass >= 0 && material->IsPassEnabled(pass)) {
			RenderSubMesh(state, entity, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(state, entity, i, material, pass);
				}
			}
		}
	}

	push_renderables->Stop();
}

void CameraInternal::RenderSubMesh(const FramebufferState& state, Entity entity, int subMeshIndex, Material material, int pass) {
	ParticleSystem p = entity->GetParticleSystem();
	uint instance = p ? p->GetParticlesCount() : 0;
	pipeline_->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, state, entity->GetTransform()->GetLocalToWorldMatrix(), instance);
}
