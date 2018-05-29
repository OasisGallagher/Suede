#include <glm/gtx/transform.hpp>

#include "time2.h"
#include "world.h"
#include "engine.h"
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
	: EntityInternal(ObjectTypeCamera), depth_(-1), depthTextureMode_(DepthTextureModeNone), normalizedRect_(0, 0, 1, 1)
	/*, gbuffer_(nullptr) */{
	pipeline_ = MEMORY_CREATE(Pipeline);

	forward_pass = Profiler::CreateSample();
	push_renderables = Profiler::CreateSample();
	get_renderable_entities = Profiler::CreateSample();

	Engine::AddFrameEventListener(this);
	Screen::AddScreenSizeChangedListener(this);

	InitializeVariables();
	SetAspect((float)Screen::GetWidth() / Screen::GetHeight());

	CreateAuxMaterial(depthMaterial_, "builtin/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(decalMaterial_, "builtin/decal", RenderQueueOverlay - 500);
}

CameraInternal::~CameraInternal() {
	//MEMORY_RELEASE(gbuffer_);
	MEMORY_RELEASE(pipeline_);

	Engine::RemoveFrameEventListener(this);
	Screen::RemoveScreenSizeChangedListener(this);

	Profiler::ReleaseSample(forward_pass);
	Profiler::ReleaseSample(push_renderables);
	Profiler::ReleaseSample(get_renderable_entities);
}

void CameraInternal::SetDepth(int value) {
	if (depth_ != value) {
		depth_ = value;
		CameraDepthChangedEventPointer e = NewWorldEvent<CameraDepthChangedEventPointer>();
		e->entity = This<Camera>();
		WorldInstance()->FireEvent(e);
	}
}

void CameraInternal::Update() {
	// Stub: main thread only.
	ClearRenderTextures();
}

void CameraInternal::Render() {
	std::vector<Entity> entities;
	glm::mat4 worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	get_renderable_entities->Restart();
	WorldInstance()->GetVisibleEntities(entities, worldToClipMatrix);
	get_renderable_entities->Stop();

	Debug::Output("[CameraInternal::Render::get_renderable_entities]\t%.2f", get_renderable_entities->GetElapsedSeconds());

	if (renderPath_ == RenderPathForward) {
		if ((depthTextureMode_ & DepthTextureModeDepth) != 0) {
		//	ForwardDepthPass(entities);
		}
	}

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	RenderTexture target = GetActiveRenderTarget();

	Shadows::Resize(target->GetWidth(), target->GetHeight());
	Shadows::Update(suede_dynamic_cast<DirectionalLight>(forwardBase), pipeline_, entities);

	UpdateTransformsUniformBuffer();

	if (renderPath_ == RenderPathForward) {
		ForwardRendering(target, entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(target, entities, forwardBase, forwardAdd);
	}

	//  Stub: main thread only.
	pipeline_->Flush(worldToClipMatrix);

	OnPostRender();

	// Stub: main thread only.
	if (!imageEffects_.empty()) {
		OnImageEffects();
	}
}

void CameraInternal::OnScreenSizeChanged(uint width, uint height) {
	if (auxTexture1_) { auxTexture1_->Resize(width, height); }
	if (auxTexture2_) { auxTexture2_->Resize(width, height); }
	if (depthTexture_) { depthTexture_->Resize(width, height); }

	float aspect = (float)width / height;
	if (!Math::Approximately(aspect, GetAspect())) {
		SetAspect(aspect);
	}
}

void CameraInternal::OnProjectionMatrixChanged() {
	GeometryUtility::CalculateFrustumPlanes(planes_, GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
}

int CameraInternal::GetFrameEventQueue() {
	return IsMainCamera() ? std::numeric_limits<int>::max() : FrameEventListener::GetFrameEventQueue();
}

void CameraInternal::OnFrameLeave() {
	if (IsMainCamera()) {
		WorldInstance()->GetScreenRenderTarget()->BindWrite(normalizedRect_);
		OnDrawGizmos();
		WorldInstance()->GetScreenRenderTarget()->Unbind();
	}
}

bool CameraInternal::IsMainCamera() const {
	return WorldInstance()->GetMainCamera().get() == this;
}

void CameraInternal::ClearRenderTextures() {
	if (auxTexture1_) { auxTexture1_->Clear(normalizedRect_, glm::vec4(clearColor_, 1)); }
	if (auxTexture2_) { auxTexture2_->Clear(normalizedRect_, glm::vec4(0, 0, 0, 1)); }
	if (depthTexture_) { depthTexture_->Clear(Rect(0, 0, 1, 1), glm::vec4(0, 0, 0, 1)); }

	RenderTexture target = targetTexture_;
	if (!target) { target = WorldInstance()->GetScreenRenderTarget(); }
	target->Clear(normalizedRect_, glm::vec4(clearColor_, 1));
}

void CameraInternal::UpdateTransformsUniformBuffer() {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	p.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	p.cameraToClipMatrix = GetProjectionMatrix();
	p.worldToShadowMatrix = Shadows::GetWorldToShadowMatrix();

	p.cameraPosition = glm::vec4(GetTransform()->GetPosition(), 1);
	UniformBufferManager::UpdateSharedBuffer(SharedTransformsUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void CameraInternal::ForwardRendering(RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (clearType_ == ClearTypeSkybox) {
		RenderSkybox(target);
	}
	
	if (forwardBase) {
		RenderForwardBase(target, entities, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(entities, forwardAdd);
	}
	
	RenderDecals(target);
}

void CameraInternal::DeferredRendering(RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
// 	if (gbuffer_ == nullptr) {
// 		InitializeDeferredRender();
// 	}
// 
// 	RenderDeferredGeometryPass(target, entities);
}

void CameraInternal::InitializeDeferredRender() {
	/*gbuffer_ = MEMORY_CREATE(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::FindShader("builtin/gbuffer"));*/
}

void CameraInternal::RenderDeferredGeometryPass(RenderTexture target, const std::vector<Entity>& entities) {
// 	gbuffer_->Bind(GBuffer::GeometryPass);
// 
// 	for (int i = 0; i < entities.size(); ++i) {
// 		Entity entity = entities[i];
// 
// 		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
// 		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
// 		material->SetTexture(Variables::mainTexture, mainTexture);
// 		pipeline_->AddRenderable(entity->GetMesh(), deferredMaterial_, 0, target, entity->GetTransform()->GetLocalToWorldMatrix());
// 	}
// 
// 	gbuffer_->Unbind();
}

void CameraInternal::SetRect(const Rect& value) {
	if (normalizedRect_ != value) {
		ClearRenderTextures();
		normalizedRect_ = value;
	}
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
	const glm::uvec4& viewport = Framebuffer0::Get()->GetViewport();
	texture->Load(TextureFormatRgb, &data[0], ColorStreamFormatRgb, viewport.z, viewport.w);

	return texture;
}

void CameraInternal::InitializeVariables() {
	depth_ = 0;
	clearType_ = ClearTypeColor;
	renderPath_ = RenderPathForward;
}

void CameraInternal::CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue) {
	Shader shader = Resources::FindShader(shaderPath);
	material = NewMaterial();
	material->SetShader(shader);
	material->SetRenderQueue(renderQueue);
}

void CameraInternal::RenderSkybox(RenderTexture target) {
	Material skybox = WorldInstance()->GetEnvironment()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = GetTransform()->GetWorldToLocalMatrix();
		matrix[3] = glm::vec4(0, 0, 0, 1);
		pipeline_->AddRenderable(Resources::GetPrimitive(PrimitiveTypeCube), skybox, 0, target, normalizedRect_, matrix);
	}
}

RenderTexture CameraInternal::GetActiveRenderTarget() {
	if (!imageEffects_.empty()) {
		if (!auxTexture1_) {
			CreateAuxTexture1();
		}

		return auxTexture1_;
	}

	RenderTexture target = targetTexture_;
	if (!target) {
		target = WorldInstance()->GetScreenRenderTarget();
	}

	return target;
}

void CameraInternal::CreateAuxTexture1() {
	auxTexture1_ = NewRenderTexture();
	auxTexture1_->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());
}

void CameraInternal::CreateAuxTexture2() {
	auxTexture2_ = NewRenderTexture();
	auxTexture2_->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());
}

void CameraInternal::CreateDepthTexture() {
	depthTexture_ = NewRenderTexture();
	depthTexture_->Create(RenderTextureFormatDepth, Screen::GetWidth(), Screen::GetHeight());
}

void CameraInternal::UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light) {
	static SharedLightUniformBuffer p;
	p.ambientLightColor = glm::vec4(WorldInstance()->GetEnvironment()->GetAmbientColor(), 1);
	p.lightColor = glm::vec4(light->GetColor(), 1);
	p.lightPosition = glm::vec4(light->GetTransform()->GetPosition(), 1);
	p.lightDirection = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	UniformBufferManager::UpdateSharedBuffer(SharedLightUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void CameraInternal::RenderForwardBase(RenderTexture target, const std::vector<Entity>& entities, Light light) {
	// Stub: GL.
	UpdateForwardBaseLightUniformBuffer(entities, light);

	forward_pass->Restart();
	ForwardPass(target, entities);
	forward_pass->Stop();
	Debug::Output("[CameraInternal::RenderForwardBase::forward_pass]\t%.2f", forward_pass->GetElapsedSeconds());
}

void CameraInternal::RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights) {
}

void CameraInternal::ForwardDepthPass(const std::vector<Entity>& entities) {
	if (!depthTexture_) { CreateDepthTexture(); }

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipeline_->AddRenderable(entity->GetMesh(), depthMaterial_, 0, depthTexture_, Rect(0, 0, 1, 1), entity->GetTransform()->GetLocalToWorldMatrix());
	}
}

void CameraInternal::ForwardPass(RenderTexture target, const std::vector<Entity>& entities) {
	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		RenderEntity(target, entity, entity->GetRenderer());
	}

	Debug::Output("[CameraInternal::ForwardPass::push_renderables]\t%.2f", push_renderables->GetElapsedSeconds());
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

void CameraInternal::RenderDecals(RenderTexture target) {
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

		pipeline_->AddRenderable(mesh, decalMaterial, 0, target, normalizedRect_, glm::mat4(1));
	}
}

void CameraInternal::OnDrawGizmos() {
	for (int i = 0; i < gizmosPainters_.size(); ++i) {
		gizmosPainters_[i]->OnDrawGizmos();
	}

	Gizmos::Flush();
}

void CameraInternal::OnImageEffects() {
	if (!auxTexture2_) { CreateAuxTexture2(); }

	RenderTexture targets[] = { auxTexture1_, auxTexture2_ };

	int index = 1;
	for (int i = 0; i < imageEffects_.size(); ++i) {
		if (i + 1 == imageEffects_.size()) {
			targets[index] = targetTexture_;
		}

		imageEffects_[i]->OnRenderImage(targets[1 - index], targets[index], normalizedRect_);
		index = 1 - index;
	}
}

void CameraInternal::RenderEntity(RenderTexture target, Entity entity, Renderer renderer) {
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
			RenderSubMesh(target, entity, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(target, entity, i, material, pass);
				}
			}
		}
	}

	push_renderables->Stop();
}

void CameraInternal::RenderSubMesh(RenderTexture target, Entity entity, int subMeshIndex, Material material, int pass) {
	ParticleSystem p = entity->GetParticleSystem();
	uint instance = p ? p->GetParticlesCount() : 0;
	pipeline_->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, target, normalizedRect_, entity->GetTransform()->GetLocalToWorldMatrix(), instance);
}
