#include "world.h"
#include "screen.h"
#include "variables.h"
#include "resources.h"
#include "rendering.h"
#include "projector.h"
#include "imageeffect.h"
#include "debug/profiler.h"
#include "internal/rendering/shadows.h"
#include "internal/rendering/pipeline.h"
#include "internal/entities/camerainternal.h"
#include "internal/rendering/uniformbuffermanager.h"

Rendering::Rendering() : normalizedRect_(0, 0, 1, 1), depthTextureMode_(DepthTextureModeNone)
	, clearType_(ClearTypeColor), renderPath_(RenderPathForward) {
	pipeline_ = MEMORY_CREATE(Pipeline);
	culling_ = MEMORY_CREATE(CullingThreadPool);
	culling_->SetCullingListener(this);

	__working = false;

	forward_pass = Profiler::CreateSample();
	push_renderables = Profiler::CreateSample();
	get_renderable_entities = Profiler::CreateSample();

	CreateAuxMaterial(depthMaterial_, "builtin/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(decalMaterial_, "builtin/decal", RenderQueueOverlay - 500);
}

Rendering::~Rendering() {
	culling_->SetCullingListener(nullptr);

	MEMORY_RELEASE(pipeline_);
	MEMORY_RELEASE(culling_);

	Profiler::ReleaseSample(forward_pass);
	Profiler::ReleaseSample(push_renderables);
	Profiler::ReleaseSample(get_renderable_entities);
}

void Rendering::Render(const Matrices& matrices) {
	if (__working) { return; }

	__working = true;
	matrices_ = matrices;
	culling_->GetVisibleEntities(matrices_.projectionMatrix * matrices_.worldToCameraMatrix);
}

void Rendering::Resize(uint width, uint height) {
	if (auxTexture1_) { auxTexture1_->Resize(width, height); }
	if (auxTexture2_) { auxTexture2_->Resize(width, height); }
	if (depthTexture_) { depthTexture_->Resize(width, height); }
}

void Rendering::SetRect(const Rect& value) {
	if (normalizedRect_ != value) {
		ClearRenderTextures();
		normalizedRect_ = value;
	}
}

void Rendering::OnCullingFinished(Culling* worker) {
	ClearRenderTextures();

	std::vector<Entity>& entities = worker->GetEntities();
	glm::mat4 worldToClipMatrix = matrices_.projectionMatrix * matrices_.worldToCameraMatrix;
	/*get_renderable_entities->Restart();
	WorldInstance()->GetVisibleEntities(entities, worldToClipMatrix);
	get_renderable_entities->Stop();

	Debug::Output("[Rendering::Render::get_renderable_entities]\t%.2f", get_renderable_entities->GetElapsedSeconds());*/

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

	__working = false;
}

void Rendering::ClearRenderTextures() {
	if (auxTexture1_) { auxTexture1_->Clear(normalizedRect_, glm::vec4(clearColor_, 1)); }
	if (auxTexture2_) { auxTexture2_->Clear(normalizedRect_, glm::vec4(0, 0, 0, 1)); }
	if (depthTexture_) { depthTexture_->Clear(Rect(0, 0, 1, 1), glm::vec4(0, 0, 0, 1)); }

	RenderTexture target = targetTexture_;
	if (!target) { target = WorldInstance()->GetScreenRenderTarget(); }
	target->Clear(normalizedRect_, glm::vec4(clearColor_, 1));
}

void Rendering::UpdateTransformsUniformBuffer() {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = matrices_.projectionMatrix * matrices_.worldToCameraMatrix;
	p.worldToCameraMatrix = matrices_.worldToCameraMatrix;
	p.cameraToClipMatrix = matrices_.projectionMatrix;
	p.worldToShadowMatrix = Shadows::GetWorldToShadowMatrix();

	p.cameraPosition = glm::vec4(matrices_.position, 1);
	UniformBufferManager::UpdateSharedBuffer(SharedTransformsUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void Rendering::ForwardRendering(RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
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

void Rendering::DeferredRendering(RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd) {
	// 	if (gbuffer_ == nullptr) {
	// 		InitializeDeferredRender();
	// 	}
	// 
	// 	RenderDeferredGeometryPass(target, entities);
}

void Rendering::InitializeDeferredRender() {
	/*gbuffer_ = MEMORY_CREATE(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::FindShader("builtin/gbuffer"));*/
}

void Rendering::RenderDeferredGeometryPass(RenderTexture target, const std::vector<Entity>& entities) {
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

void Rendering::CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue) {
	Shader shader = Resources::FindShader(shaderPath);
	material = NewMaterial();
	material->SetShader(shader);
	material->SetRenderQueue(renderQueue);
}

void Rendering::RenderSkybox(RenderTexture target) {
	Material skybox = WorldInstance()->GetEnvironment()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = matrices_.worldToCameraMatrix;
		matrix[3] = glm::vec4(0, 0, 0, 1);
		pipeline_->AddRenderable(Resources::GetPrimitive(PrimitiveTypeCube), skybox, 0, target, normalizedRect_, matrix);
	}
}

RenderTexture Rendering::GetActiveRenderTarget() {
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

void Rendering::CreateAuxTexture1() {
	auxTexture1_ = NewRenderTexture();
	auxTexture1_->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());
}

void Rendering::CreateAuxTexture2() {
	auxTexture2_ = NewRenderTexture();
	auxTexture2_->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());
}

void Rendering::CreateDepthTexture() {
	depthTexture_ = NewRenderTexture();
	depthTexture_->Create(RenderTextureFormatDepth, Screen::GetWidth(), Screen::GetHeight());
}

void Rendering::UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light) {
	static SharedLightUniformBuffer p;
	p.ambientLightColor = glm::vec4(WorldInstance()->GetEnvironment()->GetAmbientColor(), 1);
	p.lightColor = glm::vec4(light->GetColor(), 1);
	p.lightPosition = glm::vec4(light->GetTransform()->GetPosition(), 1);
	p.lightDirection = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	UniformBufferManager::UpdateSharedBuffer(SharedLightUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void Rendering::RenderForwardBase(RenderTexture target, const std::vector<Entity>& entities, Light light) {
	// Stub: GL.
	UpdateForwardBaseLightUniformBuffer(entities, light);

	forward_pass->Restart();
	ForwardPass(target, entities);
	forward_pass->Stop();
	Debug::Output("[Rendering::RenderForwardBase::forward_pass]\t%.2f", forward_pass->GetElapsedSeconds());
}

void Rendering::RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights) {
}

void Rendering::ForwardDepthPass(const std::vector<Entity>& entities) {
	if (!depthTexture_) { CreateDepthTexture(); }

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipeline_->AddRenderable(entity->GetMesh(), depthMaterial_, 0, depthTexture_, Rect(0, 0, 1, 1), entity->GetTransform()->GetLocalToWorldMatrix());
	}
}

void Rendering::ForwardPass(RenderTexture target, const std::vector<Entity>& entities) {
	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		RenderEntity(target, entity, entity->GetRenderer());
	}

	Debug::Output("[Rendering::ForwardPass::push_renderables]\t%.2f", push_renderables->GetElapsedSeconds());
	push_renderables->Clear();
}

void Rendering::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Entity> lights;
	if (!WorldInstance()->GetEntities(ObjectTypeLights, lights, nullptr)) {
		return;
	}

	forwardBase = suede_dynamic_cast<Light>(lights.front());
	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(suede_dynamic_cast<Light>(lights[i]));
	}
}

void Rendering::OnPostRender() {

}

void Rendering::RenderDecals(RenderTexture target) {
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

void Rendering::OnImageEffects() {
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

void Rendering::RenderEntity(RenderTexture target, Entity entity, Renderer renderer) {
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

void Rendering::RenderSubMesh(RenderTexture target, Entity entity, int subMeshIndex, Material material, int pass) {
	ParticleSystem p = entity->GetParticleSystem();
	uint instance = p ? p->GetParticlesCount() : 0;
	pipeline_->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, target, normalizedRect_, entity->GetTransform()->GetLocalToWorldMatrix(), instance);
}
