#include <glm/gtx/transform.hpp>

#include "time2.h"
#include "light.h"
#include "screen.h"
#include "frustum.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "imageeffect.h"
#include "debug/profiler.h"
#include "internal/base/gbuffer.h"
#include "internal/world/uniformbuffermanager.h"
#include "internal/base/uniformbuffer.h"
#include "internal/rendering/pipeline.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/camerainternal.h"

CameraInternal::CameraInternal() 
	: EntityInternal(ObjectTypeCamera)
	, fb1_(nullptr), fb2_(nullptr), gbuffer_(nullptr) {
	frustum_ = MEMORY_CREATE(Frustum);
	pipeline_ = MEMORY_CREATE(Pipeline);

	InitializeVariables();
	CreateFramebuffers();

	CreateAuxMaterial(depthMaterial_, "buildin/shaders/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(decalMaterial_, "buildin/shaders/decal", RenderQueueOverlay - 500);
	CreateAuxMaterial(directionalLightShadowMaterial_, "buildin/shaders/directional_light_depth", RenderQueueBackground - 200);
}

CameraInternal::~CameraInternal() {
	MEMORY_RELEASE(fb1_);
	MEMORY_RELEASE(fb2_);
	MEMORY_RELEASE(frustum_);
	MEMORY_RELEASE(gbuffer_);
	MEMORY_RELEASE(pipeline_);
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
	int w = Screen::GetWidth();
	int h = Screen::GetHeight();

	Framebuffer0* fb0 = Framebuffer0::Get();
	if (w != fb0->GetViewportWidth() || h != fb0->GetViewportHeight()) {
		fb0->SetViewport(w, h);
		OnContextSizeChanged(w, h);
	}

	static SharedUBOStructs::Transforms transforms;
	transforms.worldToClipMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	transforms.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	transforms.cameraToClipMatrix = GetProjectionMatrix();
	transforms.cameraPosition = glm::vec4(GetTransform()->GetPosition(), 1);

	UniformBufferManager::UpdateSharedBuffer(SharedUBONames::Transforms, &transforms, 0, sizeof(transforms));
}

void CameraInternal::Render() {
	std::vector<Entity> entities;

	Profiler::StartSample();
	GetDrawableEntities(entities);
	Debug::Output("[collect]\t%.2f\n", Profiler::EndSample());

	Pipeline::SetCamera(dsp_cast<Camera>(shared_from_this()));
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

	UpdateTimeUBO();
	
	if (renderPath_ == RenderPathForward) {
		ForwardRendering(entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(entities, forwardBase, forwardAdd);
	}

	pipeline_->Update();

	OnPostRender();

	Pipeline::SetFramebuffer(nullptr);

	if (!imageEffects_.empty()) {
		OnImageEffects();
	}

	Pipeline::SetCamera(nullptr);
	Pipeline::SetCurrent(nullptr);
	Pipeline::SetFramebuffer(nullptr);
}

void CameraInternal::UpdateTimeUBO() {
	static SharedUBOStructs::Time time;
	time.time.x = Time::GetRealTimeSinceStartup();
	time.time.y = Time::GetDeltaTime();
	UniformBufferManager::UpdateSharedBuffer(SharedUBONames::Time, &time, 0, sizeof(time));
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

	Shader shader = NewShader();
	shader->Load("buildin/shaders/gbuffer");
	deferredMaterial_->SetShader(shader);
}

void CameraInternal::AddToPipeline(Mesh mesh, Material material, const glm::mat4& localToWorldMatrix) {
	FramebufferState state;
	Pipeline::GetFramebuffer()->SaveState(state);
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		pipeline_->AddDrawable(mesh, i, material, 0, state, localToWorldMatrix);
	}
}

void CameraInternal::RenderDeferredGeometryPass(const std::vector<Entity>& entities) {
	gbuffer_->Bind(GBuffer::GeometryPass);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];

		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
		Material material = dsp_cast<Material>(deferredMaterial_->Clone());
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
		fb2_->CreateDepthRenderbuffer();
	}
}

void CameraInternal::UpdateForwardBaseLightUBO(const std::vector<Entity>& entities, Light light) {
	static SharedUBOStructs::Light parameter;
	parameter.ambientLightColor = glm::vec4(WorldInstance()->GetEnvironment()->GetAmbientColor(), 1);
	parameter.lightColor = glm::vec4(light->GetColor(), 1);
	parameter.lightPosition = glm::vec4(light->GetTransform()->GetPosition(), 1);
	parameter.lightDirection = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	UniformBufferManager::UpdateSharedBuffer(SharedUBONames::Light, &parameter, 0, sizeof(parameter));
}

void CameraInternal::RenderForwardBase(const std::vector<Entity>& entities, Light light) {
	Profiler::StartSample();
	UpdateForwardBaseLightUBO(entities, light);
	Debug::Output("[lightparam]\t%.2f\n", Profiler::EndSample());

	Profiler::StartSample();
	ForwardPass(entities);
	Debug::Output("[pass]\t%.2f\n", Profiler::EndSample());
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
		if (!IsDrawable(entity)) {
			continue;
		}

		Material material = dsp_cast<Material>(directionalLightShadowMaterial_->Clone());
		material->SetMatrix4(Variables::localToOrthographicLightMatrix, shadowDepthMatrix);
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
		if (!IsDrawable(entity)) {
			continue;
		}

		Material material = dsp_cast<Material>(depthMaterial_->Clone());
		AddToPipeline(entity->GetMesh(), material, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	Pipeline::SetFramebuffer(nullptr);
}

uint64 push_drawables = 0;
uint64 set_opaque_material = 0;

void CameraInternal::ForwardPass(const std::vector<Entity>& entities) {
	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		if (IsDrawable(entity)) {
			RenderEntity(entity, entity->GetRenderer());
		}
	}

	Debug::Output("[opaque_mat]\t%.2f\n", float(set_opaque_material) * Profiler::GetSecondsPerTick());
	Debug::Output("[opaque_push]\t%.2f\n", float(push_drawables) * Profiler::GetSecondsPerTick());

	set_opaque_material = push_drawables = 0;
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

void CameraInternal::RenderDecals() {
	std::vector<Decal*> decals;
	WorldInstance()->GetDecals(decals);

	for (int i = 0; i < decals.size(); ++i) {
		Decal* d = decals[i];
		glm::mat4 biasMatrix = glm::translate(glm::mat4(1) , glm::vec3(0.5f)) * glm::scale(glm::mat4(1), glm::vec3(0.5f));
		Material decalMaterial = dsp_cast<Material>(decalMaterial_->Clone());

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

bool CameraInternal::IsDrawable(Entity entity) {
	return entity->GetActive() && entity->GetRenderer() && entity->GetRenderer()->GetReady() && entity->GetMesh();
}

void CameraInternal::GetDrawableEntities(std::vector<Entity>& entities) {
	WorldInstance()->GetEntities(ObjectTypeEntity, entities);
	//SortDrawableEntities(entities);
}

void CameraInternal::SortDrawableEntities(std::vector<Entity>& entities) {
	int p = 0;
	for (int i = 0; i < entities.size(); ++i) {
		Entity key = entities[i];
		if (IsDrawable(key)) {
			entities[p++] = key;
		}
	}

	entities.erase(entities.begin() + p, entities.end());
}

void CameraInternal::RenderEntity(Entity entity, Renderer renderer) {
	uint64 b = Profiler::GetTicks();
	renderer->RenderEntity(entity);
	push_drawables += Profiler::GetTicks() - b;
}

void CameraInternal::UpdateMaterial(Entity entity, const glm::mat4& worldToClipMatrix, Material material) {
	//if (pass_ >= RenderPassForwardOpaque) {
	//	material->SetMatrix4(Variables::localToShadowMatrix, worldToShadowMatrix_ * localToWorldMatrix);
	//	material->SetTexture(Variables::shadowDepthTexture, shadowTexture_);
	//}
}
