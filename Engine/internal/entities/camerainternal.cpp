#include <glm/gtx/transform.hpp>

#include "time2.h"
#include "light.h"
#include "gizmos.h"
#include "screen.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "imageeffect.h"
#include "gizmospainter.h"
#include "debug/profiler.h"
#include "geometryutility.h"
#include "internal/base/gbuffer.h"
#include "internal/base/renderdefines.h"
#include "internal/base/uniformbuffer.h"
#include "internal/rendering/pipeline.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/camerainternal.h"
#include "internal/rendering/uniformbuffermanager.h"

CameraInternal::CameraInternal()
	: EntityInternal(ObjectTypeCamera)
	, fb1_(nullptr), fb2_(nullptr), gbuffer_(nullptr) {
	frustum_ = MEMORY_CREATE(Frustum);
	frustum_->SetProjectionMatrixChangedListener(this);

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
	MEMORY_RELEASE(frustum_);
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
	UpdateTransformsUniformBuffer();
}

void CameraInternal::Render() {
	std::vector<Entity> entities;
	get_renderable_entities->Restart();
	GetRenderableEntities(entities);
	get_renderable_entities->Stop();
	Debug::Output("[CameraInternal::Render::get_renderable_entities]\t%.2f\n", get_renderable_entities->GetElapsedSeconds());

	Pipeline::SetCamera(suede_dynamic_cast<Camera>(shared_from_this()));
	Pipeline::SetCurrent(pipeline_);

	if (renderPath_ == RenderPathForward) {
		if ((depthTextureMode_ & DepthTextureModeDepth) != 0) {
		//	ForwardDepthPass(entities);
		}
	}

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	if (forwardBase) {
	//	ShadowDepthPass(entities, forwardBase);
	}

	Pipeline::SetFramebuffer(GetActiveFramebuffer());

	if (renderPath_ == RenderPathForward) {
		ForwardRendering(entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(entities, forwardBase, forwardAdd);
	}

	//  Stub: main thread only.
	pipeline_->Flush();

	OnPostRender();

	Pipeline::SetFramebuffer(nullptr);

	// Stub: main thread only.
	if (!imageEffects_.empty()) {
		OnImageEffects();
	}

	OnDrawGizmos();

	Pipeline::SetCamera(nullptr);
	Pipeline::SetCurrent(nullptr);
	Pipeline::SetFramebuffer(nullptr);
}

void CameraInternal::OnProjectionMatrixChanged() {
	GeometryUtility::CalculateFrustumPlanes(planes_, GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
}

void CameraInternal::ClearFramebuffers() {
	fb1_->Clear(FramebufferClearBitmaskColorDepth);
	if (fb2_ != nullptr) {
		fb2_->Clear(FramebufferClearBitmaskColorDepth);
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

void CameraInternal::UpdateTransformsUniformBuffer() {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	p.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	p.cameraToClipMatrix = GetProjectionMatrix();
	p.cameraPosition = glm::vec4(GetTransform()->GetPosition(), 1);
	UniformBufferManager::UpdateSharedBuffer(SharedTransformsUniformBuffer::GetName(), &p, 0, sizeof(p));
}

bool CameraInternal::GetPerspective() const {
	return frustum_->GetPerspective();
}

void CameraInternal::SetPerspective(bool value) {
	frustum_->SetPerspective(value);
}

float CameraInternal::GetOrthographicSize() const {
	return frustum_->GetOrthographicSize();
}

void CameraInternal::SetOrthographicSize(float value) {
	frustum_->SetOrthographicSize(value);
}

void CameraInternal::SetAspect(float value) {
	frustum_->SetAspect(value);
}

void CameraInternal::SetNearClipPlane(float value) {
	frustum_->SetNearClipPlane(value);
}

void CameraInternal::SetFarClipPlane(float value) {
	frustum_->SetFarClipPlane(value);
}

void CameraInternal::SetFieldOfView(float value) {
	frustum_->SetFieldOfView(value);
}

float CameraInternal::GetAspect() {
	return frustum_->GetAspect();
}

float CameraInternal::GetNearClipPlane() {
	return frustum_->GetNearClipPlane();
}

float CameraInternal::GetFarClipPlane() {
	return frustum_->GetFarClipPlane();
}

float CameraInternal::GetFieldOfView() {
	return frustum_->GetFieldOfView();
}

const glm::mat4 & CameraInternal::GetProjectionMatrix() {
	return frustum_->GetProjectionMatrix();
}

void CameraInternal::ForwardRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (clearType_ == ClearTypeSkybox) {
		RenderSkybox();
	}
	
	if (forwardBase) {
		RenderForwardBase(entities, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(entities, forwardAdd);
	}
	
	RenderDecals();
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
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::FindShader("builtin/gbuffer"));
}

void CameraInternal::AddToPipeline(Mesh mesh, Material material, const glm::mat4& localToWorldMatrix) {
	FramebufferState state;
	Pipeline::GetFramebuffer()->SaveState(state);
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		pipeline_->AddRenderable(mesh, i, material, 0, state, localToWorldMatrix);
	}
}

void CameraInternal::RenderDeferredGeometryPass(const std::vector<Entity>& entities) {
	gbuffer_->Bind(GBuffer::GeometryPass);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];

		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
		material->SetTexture(Variables::mainTexture, mainTexture);
		AddToPipeline(entity->GetMesh(), deferredMaterial_, entity->GetTransform()->GetLocalToWorldMatrix());
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

	depthTexture_ = NewRenderTexture();
	depthTexture_->Load(RenderTextureFormatDepth, w, h);

	shadowTexture_ = NewRenderTexture();
	shadowTexture_->Load(RenderTextureFormatShadow, w, h);
}

void CameraInternal::CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue) {
	Shader shader = Resources::FindShader(shaderPath);
	material = NewMaterial();
	material->SetShader(shader);
	material->SetRenderQueue(renderQueue);
}

void CameraInternal::RenderSkybox() {
	Material skybox = WorldInstance()->GetEnvironment()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = GetTransform()->GetWorldToLocalMatrix();
		matrix[3] = glm::vec4(0, 0, 0, 1);
		AddToPipeline(Resources::GetPrimitive(PrimitiveTypeCube), skybox, matrix);
	}
}

void CameraInternal::OnViewportSizeChanged(int w, int h) {
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

void CameraInternal::RenderForwardBase(const std::vector<Entity>& entities, Light light) {
	// Stub: GL.
	UpdateForwardBaseLightUniformBuffer(entities, light);

	forward_pass->Restart();
	ForwardPass(entities);
	forward_pass->Stop();
	Debug::Output("[CameraInternal::RenderForwardBase::forward_pass]\t%.2f\n", forward_pass->GetElapsedSeconds());
}

void CameraInternal::ShadowDepthPass(const std::vector<Entity>& entities, Light light) {
	if (light->GetType() != ObjectTypeDirectionalLight) {
		Debug::LogError("invalid light type");
		return;
	}

	fb1_->SetDepthTexture(shadowTexture_);
	Pipeline::SetFramebuffer(fb1_);

	glm::vec3 lightPosition = light->GetTransform()->GetRotation() * glm::vec3(0, 0, 1);
	glm::mat4 projection = glm::ortho(-100.f, 100.f, -100.f, 100.f, -100.f, 100.f);
	glm::mat4 view = glm::lookAt(lightPosition * 10.f, glm::vec3(0), light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		directionalLightShadowMaterial_->SetMatrix4(Variables::localToOrthographicLightMatrix, shadowDepthMatrix);
		AddToPipeline(entity->GetMesh(), directionalLightShadowMaterial_, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	glm::mat4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	worldToShadowMatrix_ = bias * shadowDepthMatrix;

	Pipeline::SetFramebuffer(nullptr);
}

void CameraInternal::RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights) {
}

void CameraInternal::ForwardDepthPass(const std::vector<Entity>& entities) {
	fb1_->SetDepthTexture(depthTexture_);
	Pipeline::SetFramebuffer(fb1_);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		AddToPipeline(entity->GetMesh(), depthMaterial_, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	Pipeline::SetFramebuffer(nullptr);
}

void CameraInternal::ForwardPass(const std::vector<Entity>& entities) {
	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		RenderEntity(entity, entity->GetRenderer());
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

void CameraInternal::RenderDecals() {
	std::vector<Decal*> decals;
	WorldInstance()->GetDecals(decals);

	for (int i = 0; i < decals.size(); ++i) {
		Decal* d = decals[i];
		glm::mat4 biasMatrix = glm::translate(glm::mat4(1) , glm::vec3(0.5f)) * glm::scale(glm::mat4(1), glm::vec3(0.5f));
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

		AddToPipeline(mesh, decalMaterial, glm::mat4(1));
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

		Pipeline::SetFramebuffer(active);
		imageEffects_[i]->OnRenderImage(textures[1 - index], textures[index]);

		index = 1 - index;
	}
}

void CameraInternal::GetRenderableEntitiesInHierarchy(std::vector<Entity>& entities, Transform root, const glm::mat4& worldToClipMatrix) {
	for (int i = 0; i < root->GetChildCount(); ++i) {
		if (root->GetEntity()->GetStatus() != EntityStatusReady) {
			Debug::Break();
		}

		Entity child = root->GetChildAt(i)->GetEntity();
		if (child->GetStatus() != EntityStatusReady) {
			continue;
		}

		if (!IsVisible(child, worldToClipMatrix)) {
			continue;
		}

		if (CheckRenderComponents(child)) {
			entities.push_back(child);
		}

		GetRenderableEntitiesInHierarchy(entities, child->GetTransform(), worldToClipMatrix);
	}
}

bool CameraInternal::CheckRenderComponents(Entity entity) {
	return entity->GetActive() && entity->GetRenderer() && entity->GetMesh();
}

bool CameraInternal::IsVisible(Entity entity, const glm::mat4& worldToClipMatrix) {
	const Bounds& bounds = entity->GetBounds();
	if (bounds.IsEmpty()) {
		return false;
	}

	return FrustumCulling(bounds, worldToClipMatrix);
}

bool CameraInternal::FrustumCulling(const Bounds& bounds, const glm::mat4& worldToClipMatrix) {
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	bool inside = false;
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix * glm::vec4(points[i], 1);
		p /= p.w;
		if (p.x >= -1 && p.x <= 1 && p.y >= -1 && p.y <= 1 && p.z >= -1 && p.z <= 1) {
			inside = true;
		}

		points[i] = glm::vec3(p);
		min = glm::min(min, points[i]);
		max = glm::max(max, points[i]);
	}

	if (inside) {
		glm::vec2 size(max.x - min.x, max.y - min.y);
		return glm::dot(size, size) > MIN_RENDERABLE_RADIUS_SQUARED;
	}

	return false;
}

void CameraInternal::GetRenderableEntities(std::vector<Entity>& entities) {
	GetRenderableEntitiesInHierarchy(entities, 
		WorldInstance()->GetRootTransform(), 
		GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix()
	);
}

void CameraInternal::RenderEntity(Entity entity, Renderer renderer) {
	push_renderables->Start();
	renderer->RenderEntity(entity);
	push_renderables->Stop();
}

void CameraInternal::UpdateMaterial(Entity entity, const glm::mat4& worldToClipMatrix, Material material) {
	//if (pass_ >= RenderPassForwardOpaque) {
	//	material->SetMatrix4(Variables::localToShadowMatrix, worldToShadowMatrix_ * localToWorldMatrix);
	//	material->SetTexture(Variables::shadowDepthTexture, shadowTexture_);
	//}
}
