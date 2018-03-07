#include <glm/gtx/transform.hpp>

#include "time2.h"
#include "light.h"
#include "screen.h"
#include "frustum.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "imageeffect.h"
#include "internal/base/ubo.h"
#include "internal/base/gbuffer.h"
#include "internal/world/globalubo.h"
#include "internal/rendering/pipeline.h"
#include "internal/world/worldinternal.h"
#include "internal/entities/camerainternal.h"


static bool __debugIndexSet = false;

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
	transforms.worldToClipSpaceMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();
	transforms.worldToCameraSpaceMatrix = GetTransform()->GetWorldToLocalMatrix();
	transforms.cameraToClipSpaceMatrix = GetProjectionMatrix();
	transforms.cameraPosition = glm::vec4(GetTransform()->GetPosition(), 1);

	GlobalUBO::Get()->SetBuffer(SharedUBONames::Transforms , &transforms, 0, sizeof(transforms));
}

void CameraInternal::Render() {
	std::vector<Entity> entities;

	Debug::StartSample();
	GetRenderableEntities(entities);
	Debug::Output("[collect]\t%.2f\n", Debug::EndSample());

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

	Framebuffer::SetCurrentWrite(GetActiveFramebuffer());

	UpdateTimeUBO();
	
	if (renderPath_ == RenderPathForward) {
		ForwardRendering(entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(entities, forwardBase, forwardAdd);
	}

	pipeline_->Update();

	OnPostRender();

	Framebuffer::SetCurrentWrite(nullptr);

	if (!imageEffects_.empty()) {
		OnImageEffects();
	}

	Pipeline::SetCurrent(nullptr);
	pass_ = RenderPassNone;
}

void CameraInternal::UpdateTimeUBO() {
	static SharedUBOStructs::Time time;
	time.time.x = Time::GetRealTimeSinceStartup();
	time.time.y = Time::GetDeltaTime();
	GlobalUBO::Get()->SetBuffer(SharedUBONames::Time, &time, 0, sizeof(time));
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

void CameraInternal::AddToPipeline(Mesh mesh, Material material) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		Renderable* renderable = pipeline_->CreateRenderable();
		renderable->pass = 0;
		renderable->instance = 0;
		renderable->mesh = mesh;
		renderable->subMeshIndex = i;
		renderable->material = material;
		Framebuffer::GetCurrentWrite()->SaveState(renderable->state);
	}
}

void CameraInternal::RenderDeferredGeometryPass(const std::vector<Entity>& entities) {
	pass_ = RenderPassDeferredGeometryPass;
	gbuffer_->Bind(GBuffer::GeometryPass);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];

		glm::mat4 localToClipSpaceMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix() * entity->GetTransform()->GetLocalToWorldMatrix();
		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);

		Material material = dsp_cast<Material>(deferredMaterial_->Clone());
		material->SetMatrix4(Variables::localToClipSpaceMatrix, localToClipSpaceMatrix);
		material->SetMatrix4(Variables::localToWorldSpaceMatrix, entity->GetTransform()->GetLocalToWorldMatrix());
		material->SetTexture(Variables::mainTexture, mainTexture);
		AddToPipeline(entity->GetMesh(), deferredMaterial_);
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
	pass_ = RenderPassNone;
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
		skybox->SetMatrix4(Variables::localToClipSpaceMatrix, GetProjectionMatrix() * matrix);
		AddToPipeline(Resources::GetPrimitive(PrimitiveTypeCube), skybox);
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
	GlobalUBO::Get()->SetBuffer(SharedUBONames::Light, &parameter, 0, sizeof(parameter));
}

void CameraInternal::RenderForwardBase(const std::vector<Entity>& entities, Light light) {
	Debug::StartSample();
	UpdateForwardBaseLightUBO(entities, light);
	Debug::Output("[lightparam]\t%.2f\n", Debug::EndSample());

	int from = 0;
	from = ForwardBackgroundPass(entities, from);
	Debug::StartSample();
	from = ForwardOpaquePass(entities, from);
	Debug::Output("[pass]\t%.2f\n", Debug::EndSample());
	from = ForwardTransparentPass(entities, from);
}

void CameraInternal::ShadowDepthPass(const std::vector<Entity>& entities, Light light) {
	pass_ = RenderPassShadowDepth;
	if (light->GetType() != ObjectTypeDirectionalLight) {
		Debug::LogError("invalid light type");
		return;
	}

	fb1_->SetDepthTexture(shadowTexture_);
	Framebuffer::SetCurrentWrite(fb1_);

	glm::vec3 lightPosition = light->GetTransform()->GetRotation() * glm::vec3(0, 0, 1);
	glm::mat4 projection = glm::ortho(-100.f, 100.f, -100.f, 100.f, -100.f, 100.f);
	glm::mat4 view = glm::lookAt(lightPosition * 10.f, glm::vec3(0), light->GetTransform()->GetUp());
	glm::mat4 shadowDepthMatrix = projection * view;

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		if (!IsRenderable(entity)) {
			continue;
		}

		Material material = dsp_cast<Material>(directionalLightShadowMaterial_->Clone());
		material->SetMatrix4(Variables::localToOrthographicLightSpaceMatrix, shadowDepthMatrix * entity->GetTransform()->GetLocalToWorldMatrix());
		AddToPipeline(entity->GetMesh(), directionalLightShadowMaterial_);
	}

	glm::mat4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	worldToShadowSpaceMatrix_ = bias * shadowDepthMatrix;

	Framebuffer::SetCurrentWrite(nullptr);
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
	Framebuffer::SetCurrentWrite(fb1_);

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		if (!IsRenderable(entity)) {
			continue;
		}

		Material material = dsp_cast<Material>(depthMaterial_->Clone());
		material->SetMatrix4(Variables::localToClipSpaceMatrix, GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix() * entity->GetTransform()->GetLocalToWorldMatrix());

		AddToPipeline(entity->GetMesh(), material);
	}

	Framebuffer::SetCurrentWrite(nullptr);
}

#include <ctime>
clock_t mat = 0;
clock_t push = 0;

int CameraInternal::ForwardOpaquePass(const std::vector<Entity>& entities, int from) {
	pass_ = RenderPassForwardOpaque;

	int n = 0;
	std::vector<glm::mat4> matrices;
	matrices.reserve(entities.size());

	glm::mat4 worldToClipSpaceMatrix = GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix();

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		if (IsRenderable(entity)) {
			RenderEntity(entity, entity->GetRenderer(), worldToClipSpaceMatrix, matrices);
		}
	}

	__debugIndexSet = true;

	Debug::StartSample();
	// TODO: UBO size(64k).
	size_t length = matrices.size() * sizeof(glm::mat4);
	int index = 0;
	for (size_t offset = 0; offset < length; offset += 64 * 1024) {
		uint size = Math::Min(length - offset, size_t(64 * 1024));
		GlobalUBO::Get()->SetBuffer(EntityUBONames::GetEntityMatricesName(index++), (char*)&matrices[0] + offset, 0, size);
	}

	Debug::Output("[matrix_ubo]\t%.2f\n", Debug::EndSample());

	Debug::Output("[opaque_mat]\t%.2f\n", float(mat) / CLOCKS_PER_SEC);
	Debug::Output("[opaque_push]\t%.2f\n", float(push) / CLOCKS_PER_SEC);

	mat = push = 0;

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

void CameraInternal::RenderDecals() {
	std::vector<Decal*> decals;
	WorldInstance()->GetDecals(decals);

	for (int i = 0; i < decals.size(); ++i) {
		Decal* d = decals[i];
		glm::mat4 biasMatrix = glm::translate(glm::mat4(1) , glm::vec3(0.5f)) * glm::scale(glm::mat4(1), glm::vec3(0.5f));
		Material decalMaterial = dsp_cast<Material>(decalMaterial_->Clone());

		decalMaterial->SetMatrix4(Variables::decalMatrix, biasMatrix * d->matrix);
		decalMaterial->SetTexture(Variables::mainTexture, d->texture);
		decalMaterial->SetMatrix4(Variables::worldToClipSpaceMatrix, GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());

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

		AddToPipeline(mesh, decalMaterial);
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

		Framebuffer::SetCurrentWrite(active);
		imageEffects_[i]->OnRenderImage(textures[1 - index], textures[index]);

		index = 1 - index;
	}

	Framebuffer::SetCurrentWrite(nullptr);
}

bool CameraInternal::IsRenderable(Entity entity) {
	return entity->GetActive() && entity->GetRenderer()
		&& entity->GetMesh() && entity->GetMesh()->GetSubMeshCount() > 0;
}

void CameraInternal::GetRenderableEntities(std::vector<Entity>& entities) {
	WorldInstance()->GetEntities(ObjectTypeEntity, entities);
	//SortRenderableEntities(entities);
}

void CameraInternal::SortRenderableEntities(std::vector<Entity>& entities) {
	int p = 0;
	for (int i = 0; i < entities.size(); ++i) {
		Entity key = entities[i];
		if (IsRenderable(key)) {
			entities[p++] = key;
		}
	}

	entities.erase(entities.begin() + p, entities.end());
}

void CameraInternal::RenderEntity(Entity entity, Renderer renderer, const glm::mat4& worldToClipSpaceMatrix, std::vector<glm::mat4>& matrices) {
	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		//UpdateMaterial(entity, worldToClipSpaceMatrix, material);
		const uint matrixCount = (sizeof(EntityUBOStructs::EntityMatrices) / sizeof(glm::mat4));

		if (!__debugIndexSet) {
			material->SetInt("c_index", matrices.size() / matrixCount);
		}
		clock_t b = clock();

		glm::mat4 localToWorldSpaceMatrix = entity->GetTransform()->GetLocalToWorldMatrix();
		glm::mat4 localToClipSpaceMatrix = worldToClipSpaceMatrix * localToWorldSpaceMatrix;

		matrices.push_back(localToWorldSpaceMatrix);
		matrices.push_back(localToClipSpaceMatrix);
		mat += clock() - b;
	}

	clock_t b = clock();
	renderer->RenderEntity(entity);
	push += clock() - b;
}

void CameraInternal::UpdateMaterial(Entity entity, const glm::mat4& worldToClipSpaceMatrix, Material material) {
	/*glm::mat4 localToWorldSpaceMatrix = entity->GetTransform()->GetLocalToWorldMatrix();
	glm::mat4 localToClipSpaceMatrix = worldToClipSpaceMatrix * localToWorldSpaceMatrix;

	material->SetMatrix4(Variables::localToWorldSpaceMatrix, localToWorldSpaceMatrix);
	material->SetMatrix4(Variables::localToClipSpaceMatrix, localToClipSpaceMatrix);
	*/
	if (pass_ >= RenderPassForwardOpaque) {
	//	material->SetMatrix4(Variables::localToShadowSpaceMatrix, worldToShadowSpaceMatrix_ * localToWorldSpaceMatrix);
	//	material->SetTexture(Variables::shadowDepthTexture, shadowTexture_);
	}
}
