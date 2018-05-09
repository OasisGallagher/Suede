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
#include "internal/rendering/shadows.h"
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

	Screen::AddScreenSizeChangedListener(this);

	InitializeVariables();
	CreateFramebuffers();

	CreateAuxMaterial(depthMaterial_, "builtin/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(decalMaterial_, "builtin/decal", RenderQueueOverlay - 500);
}

CameraInternal::~CameraInternal() {
	MEMORY_RELEASE(fb1_);
	MEMORY_RELEASE(fb2_);
	MEMORY_RELEASE(fbDepth_);

	MEMORY_RELEASE(gbuffer_);
	MEMORY_RELEASE(pipeline_);

	Screen::RemoveScreenSizeChangedListener(this);

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

void CameraInternal::SetTargetTexture(RenderTexture value) {
	value->Resize(Screen::GetWidth(), Screen::GetHeight());
	fb1_->SetRenderTexture(FramebufferAttachment0, value);
}

RenderTexture CameraInternal::GetTargetTexture() {
	return fb1_->GetRenderTexture(FramebufferAttachment0);
}

void CameraInternal::Update() {
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

	if (WorldInstance()->GetMainCamera().get() == this) {
		Shadows::Get()->Update(suede_dynamic_cast<DirectionalLight>(forwardBase), pipeline_, entities);
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

void CameraInternal::OnScreenSizeChanged(uint width, uint height) {
	fb1_->SetViewport(width, height);

	if (fbDepth_ == nullptr) {
		fbDepth_->SetViewport(width, height);
	}

	if (fb2_ != nullptr) {
		fb2_->SetViewport(width, height);
	}

	if (renderTexture_) {
		renderTexture_->Resize(width, height);
	}

	RenderTexture target = GetTargetTexture();
	if (target && target != renderTexture_) {
		target->Resize(width, height);
	}

	depthTexture_->Resize(width, height);

	float aspect = (float)width / height;
	if (!Math::Approximately(aspect, GetAspect())) {
		SetAspect(aspect);
	}
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

void CameraInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::GetRealTimeSinceStartup();
	p.time.y = Time::GetDeltaTime();
	UniformBufferManager::UpdateSharedBuffer(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void CameraInternal::UpdateTransformsUniformBuffer() {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	p.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	p.cameraToClipMatrix = GetProjectionMatrix();
	p.worldToShadowMatrix = Shadows::Get()->GetWorldToShadowMatrix();

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

void CameraInternal::RenderDeferredGeometryPass(const FramebufferState& state, const std::vector<Entity>& entities) {
	gbuffer_->Bind(GBuffer::GeometryPass);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];

		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
		material->SetTexture(Variables::mainTexture, mainTexture);
		pipeline_->AddRenderable(entity->GetMesh(), deferredMaterial_, 0, state, entity->GetTransform()->GetLocalToWorldMatrix());
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
	uint w = Screen::GetWidth();
	uint h = Screen::GetHeight();

	fb1_ = MEMORY_CREATE(Framebuffer);
	fb1_->Create(w, h);
	fb1_->CreateDepthRenderbuffer();

	depthTexture_ = NewRenderTexture();
	depthTexture_->Load(RenderTextureFormatDepth, w, h);

	SetAspect((float)w / h);
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
		pipeline_->AddRenderable(Resources::GetPrimitive(PrimitiveTypeCube), skybox, 0, state, matrix);
	}
}

FramebufferBase* CameraInternal::GetActiveFramebuffer() {
	FramebufferBase* active = nullptr;

	if (!imageEffects_.empty()) {
		SetUpFramebuffer1();
	}

	if (fb1_->GetRenderTexture(FramebufferAttachment0)) {
		active = fb1_;
		active->SetClearColor(Framebuffer0::Get()->GetClearColor());
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
	fb2_ = MEMORY_CREATE(Framebuffer);
	fb2_->Create(fb1_->GetViewportWidth(), fb1_->GetViewportHeight());

	renderTexture2_ = NewRenderTexture();
	renderTexture2_->Load(RenderTextureFormatRgba, fb2_->GetViewportWidth(), fb2_->GetViewportHeight());
	fb2_->SetRenderTexture(FramebufferAttachment0, renderTexture2_);
	fb2_->CreateDepthRenderbuffer();
}

void CameraInternal::CreateDepthFramebuffer() {
	fbDepth_ = MEMORY_CREATE(Framebuffer);
	fbDepth_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());
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

void CameraInternal::RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights) {
}

void CameraInternal::ForwardDepthPass(const std::vector<Entity>& entities) {
	if (fbDepth_ == nullptr) { CreateDepthFramebuffer(); }

	fbDepth_->SetDepthTexture(depthTexture_);
	fbDepth_->Clear(FramebufferClearMaskDepth);

	FramebufferState state;
	fbDepth_->SaveState(state);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipeline_->AddRenderable(entity->GetMesh(), depthMaterial_, 0, state, entity->GetTransform()->GetLocalToWorldMatrix());
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

		pipeline_->AddRenderable(mesh, decalMaterial, 0, state, glm::mat4(1));
	}
}

void CameraInternal::OnDrawGizmos() {
	for (int i = 0; i < gizmosPainters_.size(); ++i) {
		gizmosPainters_[i]->OnDrawGizmos();
	}

	Gizmos::Flush();
}

void CameraInternal::OnImageEffects() {
	if (fb2_ == nullptr) { CreateFramebuffer2(); }

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
		imageEffects_[i]->OnRenderImage(textures[1 - index], textures[index]);
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
