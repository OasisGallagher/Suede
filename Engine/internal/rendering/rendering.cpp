#include "world.h"
#include "screen.h"
#include "graphics.h"
#include "profiler.h"
#include "resources.h"
#include "rendering.h"
#include "projector.h"
#include "imageeffect.h"
#include "tools/math2.h"
#include "tools/random.h"
#include "memory/memory.h"
#include "particlesystem.h"
#include "sharedtexturemanager.h"
#include "internal/base/renderdefines.h"
#include "internal/rendering/shadows.h"
#include "internal/rendering/uniformbuffermanager.h"

#define sharedSSAOTexture	SharedTextureManager::instance()->GetSSAOTexture()
#define sharedDepthTexture	SharedTextureManager::instance()->GetDepthTexture()

RenderingParameters::RenderingParameters() : normalizedRect(0, 0, 1, 1), depthTextureMode(DepthTextureMode::None)
	, clearType(ClearType::Color), renderPath(RenderPath::Forward) {
}

Rendering::Rendering(RenderingParameters* p) :p_(p) {
	CreateAuxMaterial(p_->materials.ssao, "builtin/ssao", (int)RenderQueue::Background);
	CreateAuxMaterial(p_->materials.ssaoTraversal, "builtin/ssao_traversal", (int)RenderQueue::Background);

	CreateAuxMaterial(p_->materials.depth, "builtin/depth", (int)RenderQueue::Background - 300);

	uint w = Screen::instance()->GetWidth(), h = Screen::instance()->GetHeight();

	p_->renderTextures.aux1 = NewRenderTexture();
	p_->renderTextures.aux1->Create(RenderTextureFormat::Rgba, w, h);

	p_->renderTextures.aux2 = NewRenderTexture();
	p_->renderTextures.aux2->Create(RenderTextureFormat::Rgba, w, h);

	p_->renderTextures.ssaoTraversal = NewMRTRenderTexture();
	p_->renderTextures.ssaoTraversal->Create(RenderTextureFormat::Depth, w, h);

	p_->renderTextures.ssaoTraversal->AddColorTexture(TextureFormat::Rgb32F);
	p_->renderTextures.ssaoTraversal->AddColorTexture(TextureFormat::Rgb32F);

	ssaoSample = Profiler::instance()->CreateSample();
	ssaoTraversalSample = Profiler::instance()->CreateSample();

	depthSample = Profiler::instance()->CreateSample();
	shadowSample = Profiler::instance()->CreateSample();
	renderingSample = Profiler::instance()->CreateSample();
}

void Rendering::Resize(uint width, uint height) {
	p_->renderTextures.aux1->Resize(width, height);
	p_->renderTextures.aux2->Resize(width, height);

	sharedSSAOTexture->Resize(width, height);
	sharedDepthTexture->Resize(width, height);
}

#define OutputSample(sample)	Debug::Output("%s costs %.2f ms", #sample, sample->GetElapsedSeconds() * 1000)

void Rendering::Render(RenderingPipelines& pipelines, const RenderingMatrices& matrices) {
	ClearRenderTextures();
	UpdateUniformBuffers(matrices, pipelines);

	DepthPass(pipelines);

	if (Graphics::instance()->GetAmbientOcclusionEnabled()) {
		SSAOTraversalPass(pipelines);
		SSAOPass(pipelines);
	}

	if (pipelines.forwardBaseLight) {
		ShadowPass(pipelines);
	}

	RenderPass(pipelines);

	OnPostRender();

	//Graphics::instance()->Blit(sharedSSAOTexture, nullptr);
	OnImageEffects();
}

void Rendering::ClearRenderTextures() {
	p_->renderTextures.aux1->Clear(p_->normalizedRect, p_->clearColor, 1);
	p_->renderTextures.aux2->Clear(p_->normalizedRect, Color::black, 1);
	p_->renderTextures.ssaoTraversal->Clear(p_->normalizedRect, Color::black, 1);

	sharedSSAOTexture->Clear(p_->normalizedRect, Color::white, 1);
	sharedDepthTexture->Clear(Rect(0, 0, 1, 1), Color::black, 1);

	RenderTexture target = p_->renderTextures.target;
	if (!target) { target = RenderTexture::GetDefault(); }
	target->Clear(p_->normalizedRect, p_->clearColor, 1);
}

void Rendering::UpdateTransformsUniformBuffer(const RenderingMatrices& matrices) {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = matrices.projectionMatrix * matrices.worldToCameraMatrix;
	p.worldToCameraMatrix = matrices.worldToCameraMatrix;
	p.cameraToClipMatrix = matrices.projectionMatrix;
	p.worldToShadowMatrix = Shadows::instance()->GetWorldToShadowMatrix();

	p.projParams = matrices.projParams;
	p.cameraPos = glm::vec4(matrices.cameraPos, 1);
	p.screenParams = glm::vec4(Screen::instance()->GetWidth(), Screen::instance()->GetHeight(), 0, 0);
	UniformBufferManager::instance()->Update(SharedTransformsUniformBuffer::GetName(),& p, 0, sizeof(p));
}

void Rendering::UpdateForwardBaseLightUniformBuffer(Light light) {
	static SharedLightUniformBuffer p;

	p.fogParams.color = Environment::instance()->GetFogColor();
	p.fogParams.density = Environment::instance()->GetFogDensity();

	p.ambientColor = glm::vec4(Environment::instance()->GetAmbientColor(), 1);
	
	p.lightPos = glm::vec4(light->GetTransform()->GetPosition(), 1);
	p.lightDir = glm::vec4(light->GetTransform()->GetRotation() * glm::vec3(0, 0, -1), 0);
	Color color = light->GetColor() * light->GetIntensity();
	p.lightColor = glm::vec4(color.r, color.g, color.b, 1);

	UniformBufferManager::instance()->Update(SharedLightUniformBuffer::GetName(),& p, 0, sizeof(p));
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
	auto effects = p_->camera->GetComponents<ImageEffect>();
	for (int i = 0; i < effects.size(); ++i) {
		if (i + 1 == effects.size()) {
			targets[index] = p_->renderTextures.target;
		}

		effects[i]->OnRenderImage(targets[1 - index], targets[index], p_->normalizedRect);
		index = 1 - index;
	}
}

void Rendering::SSAOPass(RenderingPipelines& pipelines) {
	ssaoSample->Restart();
	RenderTexture temp = RenderTexture::GetTemporary(RenderTextureFormat::Rgb, Screen::instance()->GetWidth(), Screen::instance()->GetHeight());

	p_->materials.ssao->SetPass(0);
	Graphics::instance()->Blit(sharedDepthTexture, temp, p_->materials.ssao, p_->normalizedRect);

	p_->materials.ssao->SetPass(1);
	Graphics::instance()->Blit(temp, sharedSSAOTexture, p_->materials.ssao, p_->normalizedRect);

	ssaoSample->Stop();
	OutputSample(ssaoSample);
}

void Rendering::SSAOTraversalPass(RenderingPipelines& pipelines) {
	ssaoTraversalSample->Restart();
	pipelines.ssaoTraversal->Run();
	ssaoTraversalSample->Stop();
	OutputSample(ssaoTraversalSample);
}

void Rendering::DepthPass(RenderingPipelines& pipelines) {
	depthSample->Restart();
	if (pipelines.depth->GetRenderableCount() > 0) {
		pipelines.depth->Run();
	}
	depthSample->Stop();
	OutputSample(depthSample);
}

void Rendering::UpdateUniformBuffers(const RenderingMatrices& matrices, RenderingPipelines& pipelines) {
	UpdateTransformsUniformBuffer(matrices);

	if (pipelines.forwardBaseLight) {
		UpdateForwardBaseLightUniformBuffer(pipelines.forwardBaseLight);
	}
}

void Rendering::ShadowPass(RenderingPipelines& pipelines) {
	RenderTexture target = pipelines.rendering->GetTargetTexture();
	Shadows::instance()->Resize(target->GetWidth(), target->GetHeight());
	Shadows::instance()->Clear();

	shadowSample->Restart();
	pipelines.shadow->Run();
	shadowSample->Stop();
	OutputSample(shadowSample);
}

void Rendering::RenderPass(RenderingPipelines& pipelines) {
	renderingSample->Restart();
	pipelines.rendering->Run();
	renderingSample->Stop();
	OutputSample(renderingSample);
}

RenderableTraits::RenderableTraits(RenderingParameters* p) : p_(p) {
	pipelines_.depth = MEMORY_NEW(Pipeline);
	pipelines_.depth->SetTargetTexture(sharedDepthTexture, Rect(0, 0, 1, 1));

	pipelines_.ssaoTraversal = MEMORY_NEW(Pipeline);
	pipelines_.ssaoTraversal->SetTargetTexture(p_->renderTextures.ssaoTraversal, Rect(0, 0, 1, 1));

	pipelines_.rendering = MEMORY_NEW(Pipeline);

	pipelines_.shadow = MEMORY_NEW(Pipeline);
	pipelines_.shadow->SetTargetTexture(Shadows::instance()->GetShadowTexture(), Rect(0, 0, 1, 1));

	InitializeSSAOKernel();

	forward_pass = Profiler::instance()->CreateSample();
	push_renderables = Profiler::instance()->CreateSample();
	get_renderable_gameObjects = Profiler::instance()->CreateSample();
}

RenderableTraits::~RenderableTraits() {
	MEMORY_DELETE(pipelines_.depth);
	MEMORY_DELETE(pipelines_.shadow);
	MEMORY_DELETE(pipelines_.rendering);
	MEMORY_DELETE(pipelines_.ssaoTraversal);

	Profiler::instance()->ReleaseSample(forward_pass);
	Profiler::instance()->ReleaseSample(push_renderables);
	Profiler::instance()->ReleaseSample(get_renderable_gameObjects);
}

void RenderableTraits::Traits(std::vector<GameObject>& gameObjects, const RenderingMatrices& matrices) {
	matrices_ = matrices;
	Clear();

	glm::mat4 worldToClipMatrix = matrices_.projectionMatrix * matrices_.worldToCameraMatrix;

	for (int i = 0; i < gameObjects.size(); ++i) {
		GameObject go = gameObjects[i];
		pipelines_.shadow->AddRenderable(go->GetComponent<IMeshFilter>()->GetMesh(), nullptr, 0, go->GetTransform()->GetLocalToWorldMatrix());
	}

	pipelines_.shadow->Sort(SortModeMesh, worldToClipMatrix);

	bool depthPass = false;
	if (p_->renderPath == +RenderPath::Forward) {
		if ((p_->depthTextureMode & DepthTextureMode::Depth) != 0) {
			depthPass = true;
		}
	}

	if (Graphics::instance()->GetAmbientOcclusionEnabled()) {
		*pipelines_.ssaoTraversal = *pipelines_.shadow;
		SSAOPass(pipelines_.ssaoTraversal);
		depthPass = true;
	}

	if (depthPass) {
		*pipelines_.depth = *pipelines_.shadow;
		ForwardDepthPass(pipelines_.depth);
	}

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	RenderTexture target = GetActiveRenderTarget();

	if (forwardBase) {
		Shadows::instance()->Update(forwardBase, pipelines_.shadow);
	}

	pipelines_.rendering->SetTargetTexture(target, p_->normalizedRect);
	if (p_->renderPath == +RenderPath::Forward) {
		ForwardRendering(pipelines_.rendering, gameObjects, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(pipelines_.rendering, gameObjects, forwardBase, forwardAdd);
	}

	pipelines_.rendering->Sort(SortModeMeshMaterial, worldToClipMatrix);
}

void RenderableTraits::ForwardRendering(Pipeline* pl, const std::vector<GameObject>& gameObjects, Light forwardBase, const std::vector<Light>& forwardAdd) {
	if (p_->clearType == +ClearType::Skybox) {
		RenderSkybox(pl);
	}

	pipelines_.forwardBaseLight = forwardBase;
	if (forwardBase) {
		RenderForwardBase(pl, gameObjects, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(pl, gameObjects, forwardAdd);
	}

	RenderDecals(pl);
}

void RenderableTraits::DeferredRendering(Pipeline* pl, const std::vector<GameObject>& gameObjects, Light forwardBase, const std::vector<Light>& forwardAdd) {
	// 	if (gbuffer_ == nullptr) {
	// 		InitializeDeferredRender();
	// 	}
	// 
	// 	RenderDeferredGeometryPass(target, gameObjects);
}

void RenderableTraits::InitializeDeferredRender() {
	/*gbuffer_ = MEMORY_NEW(GBuffer);
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = NewMaterial();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Resources::instance()->FindShader("builtin/gbuffer"));*/
}

void RenderableTraits::RenderDeferredGeometryPass(Pipeline* pl, const std::vector<GameObject>& gameObjects) {
	// 	gbuffer_->Bind(GBuffer::GeometryPass);
	// 
	// 	for (int i = 0; i < gameObjects.size(); ++i) {
	// 		GameObject go = gameObjects[i];
	// 
	// 		Texture mainTexture = go->GetRenderer()->GetMaterial(0)->GetTexture(BuiltinProperties::MainTexture);
	// 		Material material = suede_dynamic_cast<Material>(deferredMaterial_->Clone());
	// 		material->SetTexture(BuiltinProperties::MainTexture, mainTexture);
	// 		pipeline_->AddRenderable(go->GetMesh(), deferredMaterial_, 0, target, go->GetTransform()->GetLocalToWorldMatrix());
	// 	}
	// 
	// 	gbuffer_->Unbind();
}

void RenderableTraits::RenderSkybox(Pipeline* pl) {
	Material skybox = Environment::instance()->GetSkybox();
	if (skybox) {
		glm::mat4 matrix = matrices_.worldToCameraMatrix;
		matrix[3] = glm::vec4(0, 0, 0, 1);
		pl->AddRenderable(Resources::instance()->GetPrimitive(PrimitiveType::Cube), skybox, 0, matrix);
	}
}

RenderTexture RenderableTraits::GetActiveRenderTarget() {
	if (!p_->camera->GetComponents<ImageEffect>().empty()) {
		return p_->renderTextures.aux1;
	}

	RenderTexture target = p_->renderTextures.target;

	if (!target) {
		target = RenderTexture::GetDefault();
	}

	return target;
}

void RenderableTraits::RenderForwardBase(Pipeline* pl, const std::vector<GameObject>& gameObjects, Light light) {
	forward_pass->Restart();
	ForwardPass(pl, gameObjects);
	forward_pass->Stop();
	Debug::Output("[RenderableTraits::RenderForwardBase::forward_pass]\t%.2f", forward_pass->GetElapsedSeconds());
}

void RenderableTraits::RenderForwardAdd(Pipeline* pl, const std::vector<GameObject>& gameObjects, const std::vector<Light>& lights) {
}

void RenderableTraits::InitializeSSAOKernel() {
	glm::vec3 kernel[SSAO_KERNEL_SIZE];
	for (int i = 0; i < SUEDE_COUNTOF(kernel); ++i) {
		float scale = float(i) / SUEDE_COUNTOF(kernel);
		scale = Math::Lerp(0.1f, 1.f, scale * scale);

		glm::vec3 sample = glm::vec3(Random::FloatRange(-1.f, 1.f), Random::FloatRange(-1.f, 1.f), Random::FloatRange(0.f, 1.f));
		sample = glm::normalize(sample);
		sample *= Random::FloatRange(0.f, 1.f);
		kernel[i] = sample * scale;
	}

	glm::vec3 noise[4 * 4];
	for (int i = 0; i < SUEDE_COUNTOF(noise); ++i) {
		noise[i] = glm::vec3(Random::FloatRange(-1.f, 1.f), Random::FloatRange(-1.f, 1.f), 0);
	}

	Texture2D noiseTexture = NewTexture2D();
	noiseTexture->Create(TextureFormat::Rgb32F, &noise, ColorStreamFormat::RgbF, 4, 4, 4);
	noiseTexture->SetWrapModeS(TextureWrapMode::Repeat);
	noiseTexture->SetWrapModeT(TextureWrapMode::Repeat);

	p_->materials.ssao->SetVector3Array("ssaoKernel", kernel, SSAO_KERNEL_SIZE);

	p_->materials.ssao->SetTexture("noiseTexture", noiseTexture);
	p_->materials.ssao->SetTexture("posTexture", p_->renderTextures.ssaoTraversal->GetColorTexture(0));
	p_->materials.ssao->SetTexture("normalTexture", p_->renderTextures.ssaoTraversal->GetColorTexture(1));
}

void RenderableTraits::SSAOPass(Pipeline* pl) {
	ReplaceMaterials(pl, p_->materials.ssaoTraversal);
}

void RenderableTraits::ForwardDepthPass(Pipeline* pl) {
	ReplaceMaterials(pl, p_->materials.depth);
}

void RenderableTraits::ForwardPass(Pipeline* pl, const std::vector<GameObject>& gameObjects) {
	for (int i = 0; i < gameObjects.size(); ++i) {
		GameObject go = gameObjects[i];
		RenderGameObject(pl, go, go->GetComponent<IRenderer>());
	}

	Debug::Output("[RenderableTraits::ForwardPass::push_renderables]\t%.2f", push_renderables->GetElapsedSeconds());
	push_renderables->Reset();
}

void RenderableTraits::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Light> lights = World::instance()->GetComponents<ILight>();
	if (lights.empty()) {
		return;
	}

	Light first = lights.front();
	if (first->GetType() == LightType::Directional) {
		forwardBase = first;
	}

	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(suede_dynamic_cast<Light>(lights[i]));
	}
}

void RenderableTraits::RenderDecals(Pipeline* pl) {
	std::vector<Decal> decals;
	World::instance()->GetDecals(decals);

	for (Decal& d : decals) {
		pl->AddRenderable(d.mesh, d.material, 0, glm::mat4(1));
	}
}

void RenderableTraits::ReplaceMaterials(Pipeline* pl, Material material) {
	uint nrenderables = pl->GetRenderableCount();
	for (uint i = 0; i < nrenderables; ++i) {
		Renderable& renderable = pl->GetRenderable(i);
		renderable.material = material;
		renderable.instance = 0;
	}
}

void RenderableTraits::RenderGameObject(Pipeline* pl, GameObject go, Renderer renderer) {
	push_renderables->Start();

	int subMeshCount = go->GetComponent<IMeshFilter>()->GetMesh()->GetSubMeshCount();
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
			RenderSubMesh(pl, go, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(pl, go, i, material, pass);
				}
			}
		}
	}

	push_renderables->Stop();
}

void RenderableTraits::RenderSubMesh(Pipeline* pl, GameObject go, int subMeshIndex, Material material, int pass) {
	ParticleSystem p = go->GetComponent<IParticleSystem>();
	uint instance = p ? p->GetParticlesCount() : 0;
	pl->AddRenderable(go->GetComponent<IMeshFilter>()->GetMesh(), subMeshIndex, material, pass, go->GetTransform()->GetLocalToWorldMatrix(), instance);
}

void RenderableTraits::Clear() {
	pipelines_.depth->Clear();
	pipelines_.shadow->Clear();
	pipelines_.rendering->Clear();
	pipelines_.ssaoTraversal->Clear();
}
