#include "world.h"
#include "screen.h"
#include "profiler.h"
#include "variables.h"
#include "resources.h"
#include "rendering.h"
#include "projector.h"
#include "imageeffect.h"
#include "internal/rendering/shadows.h"
#include "internal/rendering/uniformbuffermanager.h"

RenderingParameters::RenderingParameters() : normalizedRect(0, 0, 1, 1), depthTextureMode(DepthTextureMode::None)
	, clearType(ClearType::Color), renderPath(RenderPath::Forward) {
}

Rendering::Rendering(RenderingParameters * p) :p_(p) {
	CreateAuxMaterial(p_->materials.depth, "builtin/depth", RenderQueueBackground - 300);
	CreateAuxMaterial(p_->materials.decal, "builtin/decal", RenderQueueOverlay - 500);

	p_->renderTextures.aux1 = NewRenderTexture();
	p_->renderTextures.aux1->Create(RenderTextureFormatRgba, Screen::instance()->GetWidth(), Screen::instance()->GetHeight());

	p_->renderTextures.aux2 = NewRenderTexture();
	p_->renderTextures.aux2->Create(RenderTextureFormatRgba, Screen::instance()->GetWidth(), Screen::instance()->GetHeight());

	p_->renderTextures.depth = NewRenderTexture();
	p_->renderTextures.depth->Create(RenderTextureFormatDepth, Screen::instance()->GetWidth(), Screen::instance()->GetHeight());

	depthSample = Profiler::instance()->CreateSample();
	shadowSample = Profiler::instance()->CreateSample();
	renderingSample = Profiler::instance()->CreateSample();
}

void Rendering::Resize(uint width, uint height) {
	p_->renderTextures.aux1->Resize(width, height);
	p_->renderTextures.aux2->Resize(width, height);
	p_->renderTextures.depth->Resize(width, height);
}

#define OutputSample(sample)	Debug::Output("%s elapsed %.2f seconds", #sample, sample->GetElapsedSeconds())

void Rendering::Render(RenderingPipelines& pipelines, const RenderingMatrices& matrices) {
	ClearRenderTextures();

	UpdateUniformBuffers(matrices, pipelines);
	
	DepthPass(pipelines);

	ShadowPass(pipelines);

	RenderPass(pipelines);

	OnPostRender();

	if (!p_->imageEffects.empty()) {
		OnImageEffects();
	}
}

void Rendering::ClearRenderTextures() {
	p_->renderTextures.aux1->Clear(p_->normalizedRect, glm::vec4(p_->clearColor, 1));
	p_->renderTextures.aux2->Clear(p_->normalizedRect, glm::vec4(0, 0, 0, 1));
	p_->renderTextures.depth->Clear(Rect(0, 0, 1, 1), glm::vec4(0, 0, 0, 1));

	RenderTexture target = p_->renderTextures.target;
	if (!target) { target = RenderTexture::GetDefault(); }
	target->Clear(p_->normalizedRect, glm::vec4(p_->clearColor, 1));
}

void Rendering::UpdateTransformsUniformBuffer(const RenderingMatrices& matrices) {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = matrices.projectionMatrix * matrices.worldToCameraMatrix;
	p.worldToCameraMatrix = matrices.worldToCameraMatrix;
	p.cameraToClipMatrix = matrices.projectionMatrix;
	p.worldToShadowMatrix = Shadows::instance()->GetWorldToShadowMatrix();

	p.cameraPos = glm::vec4(matrices.position, 1);
	UniformBufferManager::instance()->UpdateSharedBuffer(SharedTransformsUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void Rendering::UpdateForwardBaseLightUniformBuffer(Light light) {
	static SharedLightUniformBuffer p;

	p.fog.color = Environment::instance()->GetFogColor();
	p.fog.density = Environment::instance()->GetFogDensity();

	p.ambientColor = glm::vec4(Environment::instance()->GetAmbientColor(), 1);
	
	p.lightPos = glm::vec4(light->GetTransform()->GetPosition(), 1);
	p.lightDir = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	p.lightColor = glm::vec4(light->GetColor() * light->GetIntensity(), 1);

	UniformBufferManager::instance()->UpdateSharedBuffer(SharedLightUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void Rendering::CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue) {
	Shader shader = Resources::instance()->FindShader(shaderPath);
	material = NewMaterial();
	material->SetShader(shader);
	material->SetRenderQueue(renderQueue);
}

void Rendering::OnPostRender() {

}

void Rendering::OnImageEffects() {
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

void Rendering::DepthPass(RenderingPipelines &pipelines) {
	depthSample->Restart();
	if (pipelines.depth->GetRenderableCount() > 0) {
		pipelines.depth->Run();
	}
	depthSample->Stop();
	OutputSample(depthSample);
}

void Rendering::UpdateUniformBuffers(const RenderingMatrices& matrices, RenderingPipelines &pipelines) {
	UpdateTransformsUniformBuffer(matrices);

	if (pipelines.forwardBaseLight) {
		UpdateForwardBaseLightUniformBuffer(pipelines.forwardBaseLight);
	}
}

void Rendering::ShadowPass(RenderingPipelines &pipelines) {
	RenderTexture target = pipelines.rendering->GetTargetTexture();
	Shadows::instance()->Resize(target->GetWidth(), target->GetHeight());
	Shadows::instance()->Clear();

	shadowSample->Restart();
	pipelines.shadow->Run();
	shadowSample->Stop();
	OutputSample(shadowSample);
}

void Rendering::RenderPass(RenderingPipelines &pipelines) {
	renderingSample->Restart();
	pipelines.rendering->Run(true);
	renderingSample->Stop();
	OutputSample(renderingSample);
}

RenderableTraits::RenderableTraits(RenderingParameters* p/*RenderingListener* listener*/) : p_(p)/*, listener_(listener)*/ {
	pipelines_.depth = MEMORY_NEW(Pipeline);
	pipelines_.rendering = MEMORY_NEW(Pipeline);

	pipelines_.shadow = MEMORY_NEW(Pipeline);
	pipelines_.shadow->SetTargetTexture(Shadows::instance()->GetShadowTexture(), Rect(0, 0, 1, 1));

	forward_pass = Profiler::instance()->CreateSample();
	push_renderables = Profiler::instance()->CreateSample();
	get_renderable_entities = Profiler::instance()->CreateSample();
}

RenderableTraits::~RenderableTraits() {
	MEMORY_DELETE(pipelines_.depth);
	MEMORY_DELETE(pipelines_.shadow);
	MEMORY_DELETE(pipelines_.rendering);

	Profiler::instance()->ReleaseSample(forward_pass);
	Profiler::instance()->ReleaseSample(push_renderables);
	Profiler::instance()->ReleaseSample(get_renderable_entities);
}

void RenderableTraits::Traits(std::vector<Entity>& entities, const RenderingMatrices& matrices) {
	matrices_ = matrices;
	Clear();


	glm::mat4 worldToClipMatrix = matrices_.projectionMatrix * matrices_.worldToCameraMatrix;

	for (int i = 0; i < entities.size(); ++i) {
		Entity entity = entities[i];
		pipelines_.shadow->AddRenderable(entity->GetMesh(), nullptr, 0, entity->GetTransform()->GetLocalToWorldMatrix());
	}

	pipelines_.shadow->Sort(SortModeMesh, worldToClipMatrix);

	if (p_->renderPath == +RenderPath::Forward) {
		if ((p_->depthTextureMode & DepthTextureMode::Depth) != 0) {
			*pipelines_.depth = *pipelines_.shadow;
			pipelines_.depth->SetTargetTexture(p_->renderTextures.depth, Rect(0, 0, 1, 1));

			ForwardDepthPass(pipelines_.depth);
		}
	}

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	RenderTexture target = GetActiveRenderTarget();

	if (forwardBase) {
		Shadows::instance()->Update(suede_dynamic_cast<DirectionalLight>(forwardBase), pipelines_.shadow);
	}

	pipelines_.rendering->SetTargetTexture(target, p_->normalizedRect);
	if (p_->renderPath == +RenderPath::Forward) {
		ForwardRendering(pipelines_.rendering, entities, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(pipelines_.rendering, entities, forwardBase, forwardAdd);
	}

	pipelines_.rendering->Sort(SortModeMeshMaterial, worldToClipMatrix);
}

void RenderableTraits::ForwardRendering(Pipeline* pl, const std::vector<Entity>& entities_, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (p_->clearType == +ClearType::Skybox) {
		RenderSkybox(pl);
	}

	if (forwardBase) {
		RenderForwardBase(pl, entities_, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(pl, entities_, forwardAdd);
	}

	RenderDecals(pl);
}

void RenderableTraits::DeferredRendering(Pipeline* pl, const std::vector<Entity>& entities_, Light forwardBase, const std::vector<Light>& forwardAdd) {
	// 	if (gbuffer_ == nullptr) {
	// 		InitializeDeferredRender();
	// 	}
	// 
	// 	RenderDeferredGeometryPass(target, entities_);
}

void RenderableTraits::InitializeDeferredRender() {
	/*gbuffer_ = MEMORY_NEW(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::instance()->FindShader("builtin/gbuffer"));*/
}

void RenderableTraits::RenderDeferredGeometryPass(Pipeline* pl, const std::vector<Entity>& entities_) {
	// 	gbuffer_->Bind(GBuffer::GeometryPass);
	// 
	// 	for (int i = 0; i < entities_.size(); ++i) {
	// 		Entity entity = entities_[i];
	// 
	// 		Texture mainTexture = entity->GetRenderer()->GetMaterial(0)->GetTexture(Variables::MainTexture);
	// 		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
	// 		material->SetTexture(Variables::MainTexture, mainTexture);
	// 		pipeline_->AddRenderable(entity->GetMesh(), deferredMaterial_, 0, target, entity->GetTransform()->GetLocalToWorldMatrix());
	// 	}
	// 
	// 	gbuffer_->Unbind();
}

void RenderableTraits::RenderSkybox(Pipeline* pl) {
	Material skybox = Environment::instance()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = matrices_.worldToCameraMatrix;
		matrix[3] = glm::vec4(0, 0, 0, 1);
		pl->AddRenderable(Resources::instance()->GetPrimitive(PrimitiveTypeCube), skybox, 0, matrix);
	}
}

RenderTexture RenderableTraits::GetActiveRenderTarget() {
	if (!p_->imageEffects.empty()) {
		return p_->renderTextures.aux1;
	}

	RenderTexture target = p_->renderTextures.target;
	if (!target) {
		target = RenderTexture::GetDefault();
	}

	return target;
}

void RenderableTraits::RenderForwardBase(Pipeline* pl, const std::vector<Entity>& entities_, Light light) {
	pipelines_.forwardBaseLight = light;

	forward_pass->Restart();
	ForwardPass(pl, entities_);
	forward_pass->Stop();
	Debug::Output("[RenderableTraits::RenderForwardBase::forward_pass]\t%.2f", forward_pass->GetElapsedSeconds());
}

void RenderableTraits::RenderForwardAdd(Pipeline* pl, const std::vector<Entity>& entities_, const std::vector<Light>& lights) {
}

void RenderableTraits::ForwardDepthPass(Pipeline* pl) {
	uint nrenderables = pl->GetRenderableCount();
	for (uint i = 0; i < nrenderables; ++i) {
		Renderable& renderable = pl->GetRenderable(i);
		renderable.material = p_->materials.depth;
		renderable.instance = 0;
	}
}

void RenderableTraits::ForwardPass(Pipeline* pl, const std::vector<Entity>& entities_) {
	for (int i = 0; i < entities_.size(); ++i) {
		Entity entity = entities_[i];
		RenderEntity(pl, entity, entity->GetRenderer());
	}

	Debug::Output("[RenderableTraits::ForwardPass::push_renderables]\t%.2f", push_renderables->GetElapsedSeconds());
	push_renderables->Reset();
}

void RenderableTraits::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Entity> lights;
	if (!World::instance()->GetEntities(ObjectTypeLights, lights)) {
		return;
	}

	forwardBase = suede_dynamic_cast<Light>(lights.front());
	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(suede_dynamic_cast<Light>(lights[i]));
	}
}

void RenderableTraits::RenderDecals(Pipeline* pl) {
	std::vector<Decal*> decals;
	World::instance()->GetDecals(decals);

	for (int i = 0; i < decals.size(); ++i) {
		Decal* d = decals[i];
		glm::mat4 biasMatrix = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.5f)), glm::vec3(0.5f));
		Material decalMaterial = suede_dynamic_cast<Material>(p_->materials.decal->Clone());

		decalMaterial->SetMatrix4(Variables::DecalMatrix, biasMatrix * d->matrix);
		decalMaterial->SetTexture(Variables::MainTexture, d->texture);

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

		pl->AddRenderable(mesh, decalMaterial, 0, glm::mat4(1));
	}
}

void RenderableTraits::RenderEntity(Pipeline* pl, Entity entity, Renderer renderer) {
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
			RenderSubMesh(pl, entity, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(pl, entity, i, material, pass);
				}
			}
		}
	}

	push_renderables->Stop();
}

void RenderableTraits::RenderSubMesh(Pipeline* pl, Entity entity, int subMeshIndex, Material material, int pass) {
	ParticleSystem p = entity->GetParticleSystem();
	uint instance = p ? p->GetParticlesCount() : 0;
	pl->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, entity->GetTransform()->GetLocalToWorldMatrix(), instance);
}

void RenderableTraits::Clear() {
	pipelines_.depth->Clear();
	pipelines_.shadow->Clear();
	pipelines_.rendering->Clear();
}
