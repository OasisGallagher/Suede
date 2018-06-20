#include "world.h"
#include "screen.h"
#include "variables.h"
#include "resources.h"
#include "rendering.h"
#include "projector.h"
#include "imageeffect.h"
#include "debug/profiler.h"
#include "internal/rendering/shadows.h"
#include "internal/rendering/uniformbuffermanager.h"

RenderingParameters::RenderingParameters() : normalizedRect(0, 0, 1, 1), depthTextureMode(DepthTextureModeNone)
	, clearType(ClearTypeColor), renderPath(RenderPathForward) {
}

RenderingThread::RenderingThread(RenderingParameters * p) :p_(p) {
	CreateAuxMaterial(p_->materials.depth, "builtin/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(p_->materials.decal, "builtin/decal", RenderQueueOverlay - 500);

	p_->renderTextures.aux1 = NewRenderTexture();
	p_->renderTextures.aux1->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());

	p_->renderTextures.aux2 = NewRenderTexture();
	p_->renderTextures.aux2->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());

	p_->renderTextures.depth = NewRenderTexture();
	p_->renderTextures.depth->Create(RenderTextureFormatDepth, Screen::GetWidth(), Screen::GetHeight());
}

void RenderingThread::Resize(uint width, uint height) {
	p_->renderTextures.aux1->Resize(width, height);
	p_->renderTextures.aux2->Resize(width, height);
	p_->renderTextures.depth->Resize(width, height);
}

Sample* depthSample = Profiler::CreateSample();
Sample* shadowSample = Profiler::CreateSample();
Sample* renderingSample = Profiler::CreateSample();
#define OutputSample(sample)	Debug::Output("%s elapsed %.2f seconds", #sample, sample->GetElapsedSeconds())

void RenderingThread::Render(RenderingPipelines& pipelines, const RenderingMatrices& matrices) {
	ClearRenderTextures();

	glm::mat4 worldToClipMatrix = matrices.projectionMatrix * matrices.worldToCameraMatrix;
	depthSample->Restart();
	if (pipelines.depth->GetRenderableCount() > 0) {
		pipelines.depth->Run(worldToClipMatrix);
	}
	depthSample->Stop();
	OutputSample(depthSample);

	UpdateTransformsUniformBuffer(matrices);
	UpdateForwardBaseLightUniformBuffer(pipelines.forwardBaseLight);

	Shadows::Clear();
	shadowSample->Restart();
	pipelines.shadow->Run(worldToClipMatrix);
	shadowSample->Stop();
	OutputSample(shadowSample);

	renderingSample->Restart();
	pipelines.rendering->Run(worldToClipMatrix);
	renderingSample->Stop();
	OutputSample(renderingSample);

	OnPostRender();

	if (!p_->imageEffects.empty()) {
		OnImageEffects();
	}
}

void RenderingThread::ClearRenderTextures() {
	p_->renderTextures.aux1->Clear(p_->normalizedRect, glm::vec4(p_->clearColor, 1));
	p_->renderTextures.aux2->Clear(p_->normalizedRect, glm::vec4(0, 0, 0, 1));
	p_->renderTextures.depth->Clear(Rect(0, 0, 1, 1), glm::vec4(0, 0, 0, 1));

	RenderTexture target = p_->renderTextures.target;
	if (!target) { target = WorldInstance()->GetScreenRenderTarget(); }
	target->Clear(p_->normalizedRect, glm::vec4(p_->clearColor, 1));
}

void RenderingThread::UpdateTransformsUniformBuffer(const RenderingMatrices& matrices) {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = matrices.projectionMatrix * matrices.worldToCameraMatrix;
	p.worldToCameraMatrix = matrices.worldToCameraMatrix;
	p.cameraToClipMatrix = matrices.projectionMatrix;
	p.worldToShadowMatrix = Shadows::GetWorldToShadowMatrix();

	p.cameraPosition = glm::vec4(matrices.position, 1);
	UniformBufferManager::UpdateSharedBuffer(SharedTransformsUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void RenderingThread::UpdateForwardBaseLightUniformBuffer(Light light) {
	static SharedLightUniformBuffer p;
	p.ambientLightColor = glm::vec4(WorldInstance()->GetEnvironment()->GetAmbientColor(), 1);
	p.lightColor = glm::vec4(light->GetColor(), 1);
	p.lightPosition = glm::vec4(light->GetTransform()->GetPosition(), 1);
	p.lightDirection = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	UniformBufferManager::UpdateSharedBuffer(SharedLightUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void RenderingThread::CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue) {
	Shader shader = Resources::FindShader(shaderPath);
	material = NewMaterial();
	material->SetShader(shader);
	material->SetRenderQueue(renderQueue);
}

void RenderingThread::OnPostRender() {

}

void RenderingThread::OnImageEffects() {
	RenderTexture targets[] = { p_->renderTextures.aux1, p_->renderTextures.aux2 };

	int index = 1;
	for (int i = 0; i < p_->imageEffects.size(); ++i) {
		if (i + 1 == p_->imageEffects.size()) {
			targets[index] = p_->renderTextures.target;
		}

		p_->imageEffects[i]->OnRenderImage(targets[1 - index], targets[index], p_->normalizedRect);
		index = 1 - index;
	}
}

RenderableTraits::RenderableTraits(RenderingParameters* p/*RenderingListener* listener*/) : p_(p), status_(Waiting)/*, listener_(listener)*/ {
	pipelines_.depth = MEMORY_CREATE(Pipeline);
	pipelines_.shadow = MEMORY_CREATE(Pipeline);
	pipelines_.rendering = MEMORY_CREATE(Pipeline);

	forward_pass = Profiler::CreateSample();
	push_renderables = Profiler::CreateSample();
	get_renderable_entities = Profiler::CreateSample();
}

RenderableTraits::~RenderableTraits() {
	MEMORY_RELEASE(pipelines_.depth);
	MEMORY_RELEASE(pipelines_.shadow);
	MEMORY_RELEASE(pipelines_.rendering);

	Profiler::ReleaseSample(forward_pass);
	Profiler::ReleaseSample(push_renderables);
	Profiler::ReleaseSample(get_renderable_entities);
}

void RenderableTraits::Traits(std::vector<Entity>& entities, const RenderingMatrices& matrices) {
	matrices_ = matrices;

	pipelines_.depth->Clear();
	pipelines_.shadow->Clear();
	pipelines_.rendering->Clear();

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipelines_.shadow->AddRenderable(entity->GetMesh(), nullptr, 0, nullptr, p_->normalizedRect, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	pipelines_.shadow->Sort(SortModeMesh);

	if (p_->renderPath == RenderPathForward) {
		if ((p_->depthTextureMode & DepthTextureModeDepth) != 0) {
			*pipelines_.depth = *pipelines_.shadow;
			ForwardDepthPass(pipelines_.depth);
		}
	}

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	RenderTexture target = GetActiveRenderTarget();

	Shadows::Resize(target->GetWidth(), target->GetHeight());
	Shadows::Update(suede_dynamic_cast<DirectionalLight>(forwardBase), pipelines_.shadow);

	if (p_->renderPath == RenderPathForward) {
		ForwardRendering(pipelines_.rendering, target, entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(pipelines_.rendering, target, entities, forwardBase, forwardAdd);
	}

	pipelines_.rendering->Sort(SortModeMeshMaterial);
}

void RenderableTraits::ForwardRendering(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities_, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (p_->clearType == ClearTypeSkybox) {
		RenderSkybox(pl, target);
	}

	if (forwardBase) {
		RenderForwardBase(pl, target, entities_, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(pl, entities_, forwardAdd);
	}

	RenderDecals(pl, target);
}

void RenderableTraits::DeferredRendering(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities_, Light forwardBase, const std::vector<Light>& forwardAdd) {
	// 	if (gbuffer_ == nullptr) {
	// 		InitializeDeferredRender();
	// 	}
	// 
	// 	RenderDeferredGeometryPass(target, entities_);
}

void RenderableTraits::InitializeDeferredRender() {
	/*gbuffer_ = MEMORY_CREATE(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::FindShader("builtin/gbuffer"));*/
}

void RenderableTraits::RenderDeferredGeometryPass(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities_) {
	// 	gbuffer_->Bind(GBuffer::GeometryPass);
	// 
	// 	for (int i = 0; i < entities_.size(); ++i) {
	// 		Entity entity = entities_[i];
	// 
	// 		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::mainTexture);
	// 		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
	// 		material->SetTexture(Variables::mainTexture, mainTexture);
	// 		pipeline_->AddRenderable(entity->GetMesh(), deferredMaterial_, 0, target, entity->GetTransform()->GetLocalToWorldMatrix());
	// 	}
	// 
	// 	gbuffer_->Unbind();
}

void RenderableTraits::RenderSkybox(Pipeline* pl, RenderTexture target) {
	Material skybox = WorldInstance()->GetEnvironment()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = matrices_.worldToCameraMatrix;
		matrix[3] = glm::vec4(0, 0, 0, 1);
		pl->AddRenderable(Resources::GetPrimitive(PrimitiveTypeCube), skybox, 0, target, p_->normalizedRect, matrix);
	}
}

RenderTexture RenderableTraits::GetActiveRenderTarget() {
	if (!p_->imageEffects.empty()) {
		return p_->renderTextures.aux1;
	}

	RenderTexture target = p_->renderTextures.target;
	if (!target) {
		target = WorldInstance()->GetScreenRenderTarget();
	}

	return target;
}

void RenderableTraits::RenderForwardBase(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities_, Light light) {
	pipelines_.forwardBaseLight = light;

	forward_pass->Restart();
	ForwardPass(pl, target, entities_);
	forward_pass->Stop();
	Debug::Output("[RenderableTraits::RenderForwardBase::forward_pass]\t%.2f", forward_pass->GetElapsedSeconds());
}

void RenderableTraits::RenderForwardAdd(Pipeline* pl, const std::vector<Entity>& entities_, const std::vector<Light>& lights) {
}

void RenderableTraits::ForwardDepthPass(Pipeline* pl) {
	Rect rect(0, 0, 1, 1);
	uint nrenderables = pl->GetRenderableCount();
	for (uint i = 0; i < nrenderables; ++i) {
		Renderable& renderable = pl->GetRenderable(i);
		renderable.material = p_->materials.depth;
		renderable.target = p_->renderTextures.depth;
		renderable.instance = 0;
		renderable.normalizedRect = rect;
	}
}

void RenderableTraits::ForwardPass(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities_) {
	for (int i = 0; i < entities_.size(); ++i) {
		Entity entity = entities_[i];
		RenderEntity(pl, target, entity, entity->GetRenderer());
	}

	Debug::Output("[RenderableTraits::ForwardPass::push_renderables]\t%.2f", push_renderables->GetElapsedSeconds());
	push_renderables->Reset();
}

void RenderableTraits::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Entity> lights;
	if (!WorldInstance()->GetEntities(ObjectTypeLights, lights)) {
		return;
	}

	forwardBase = suede_dynamic_cast<Light>(lights.front());
	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(suede_dynamic_cast<Light>(lights[i]));
	}
}

void RenderableTraits::RenderDecals(Pipeline* pl, RenderTexture target) {
	std::vector<Decal*> decals;
	WorldInstance()->GetDecals(decals);

	for (int i = 0; i < decals.size(); ++i) {
		Decal* d = decals[i];
		glm::mat4 biasMatrix = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.5f)), glm::vec3(0.5f));
		Material decalMaterial = suede_dynamic_cast<Material>(p_->materials.decal->Clone());

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

		pl->AddRenderable(mesh, decalMaterial, 0, target, p_->normalizedRect, glm::mat4(1));
	}
}

void RenderableTraits::RenderEntity(Pipeline* pl, RenderTexture target, Entity entity, Renderer renderer) {
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
			RenderSubMesh(pl, target, entity, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(pl, target, entity, i, material, pass);
				}
			}
		}
	}

	push_renderables->Stop();
}

void RenderableTraits::RenderSubMesh(Pipeline* pl, RenderTexture target, Entity entity, int subMeshIndex, Material material, int pass) {
	ParticleSystem p = entity->GetParticleSystem();
	uint instance = p ? p->GetParticlesCount() : 0;
	pl->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, target, p_->normalizedRect, entity->GetTransform()->GetLocalToWorldMatrix(), instance);
}
