#include "rendering.h"

#include "world.h"
#include "graphics.h"
#include "profiler.h"
#include "projector.h"
#include "imageeffect.h"
#include "particlesystem.h"

#include "internal/rendering/context.h"
#include "internal/rendering/shadowmap.h"
#include "internal/rendering/ambientocclusion.h"

RenderingPipelines::RenderingPipelines(Context* context) {
	depth = new Pipeline(context);
	depth->SetTargetTexture(context->GetUniformState()->depthTexture.get(), Rect(0, 0, 1, 1));

	rendering = new Pipeline(context);

	shadow = new Pipeline(context);
	shadow->SetTargetTexture(context->GetShadowMap()->GetTargetTexture(), Rect(0, 0, 1, 1));

	ssaoTraversal = new Pipeline(context);
	ssaoTraversal->SetTargetTexture(context->GetAmbientOcclusion()->GetTraversalRenderTexture(), Rect(0, 0, 1, 1));
}

RenderingPipelines::~RenderingPipelines() {
	delete depth;
	delete rendering;
	delete shadow;
	delete ssaoTraversal;
}

void RenderingPipelines::Clear() {
	depth->Clear();
	shadow->Clear();
	rendering->Clear();
	ssaoTraversal->Clear();
}

Rendering::Rendering(Context* context) : context_(context) {
	ssaoSample = Profiler::CreateSample();
	ssaoTraversalSample = Profiler::CreateSample();

	depthSample = Profiler::CreateSample();
	shadowSample = Profiler::CreateSample();
	renderingSample = Profiler::CreateSample();
}

#define OutputSample(sample)	Debug::Output("%s costs %.2f ms", #sample, sample->GetElapsedSeconds() * 1000)

void Rendering::Render(RenderingPipelines* pipelines, const RenderingMatrices& matrices) {
	context_->ClearFrame();
	UpdateUniformBuffers(pipelines, matrices);

	DepthPass(pipelines);

	if (Graphics::GetAmbientOcclusionEnabled()) {
		SSAOTraversalPass(pipelines);
		SSAOPass(pipelines);
	}

	FrameState* frameState = context_->GetFrameState();
	if (frameState->forwardBaseLight) {
		ShadowPass(pipelines);
	}

	RenderPass(pipelines);

	OnPostRender();

	//Graphics::Blit(sharedSSAOTexture, nullptr);

	auto effects = frameState->camera->GetComponents<ImageEffect>();
	if (!effects.empty()) { OnImageEffects(effects); }
}

void Rendering::UpdateTransformsUniformBuffer(const RenderingMatrices& matrices) {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = matrices.projectionMatrix * matrices.worldToCameraMatrix;
	p.worldToCameraMatrix = matrices.worldToCameraMatrix;
	p.cameraToClipMatrix = matrices.projectionMatrix;
	p.worldToShadowMatrix = context_->GetShadowMap()->GetWorldToShadowMatrix();

	p.projParams = matrices.projParams;
	p.cameraPos = Vector4(matrices.cameraPos.x, matrices.cameraPos.y, matrices.cameraPos.z, 1);
	p.screenParams = Vector4((float)Screen::GetWidth(), (float)Screen::GetHeight(), 0.f, 0.f);

	context_->GetSharedUniformBuffers()->UpdateUniformBuffer(SharedTransformsUniformBuffer::GetName(),& p, 0, sizeof(p));
}

void Rendering::UpdateForwardBaseLightUniformBuffer(Light* light) {
	static SharedLightUniformBuffer p;

	memcpy(&p.fogParams.color, &Environment::GetFogColor(), sizeof(p.fogParams.color));
	p.fogParams.density = Environment::GetFogDensity();

	memcpy(&p.ambientColor, &Environment::GetAmbientColor(), sizeof(p.ambientColor));
	
	Vector3 pos = light->GetTransform()->GetPosition();
	p.lightPos = Vector4(pos.x, pos.y, pos.z, 1);

	Vector3 dir = light->GetTransform()->GetRotation() * Vector3(0, 0, -1);
	p.lightDir = Vector4(dir.x, dir.y, dir.z, 0);
	Color color = light->GetColor() * light->GetIntensity();
	p.lightColor = Vector4(color.r, color.g, color.b, 1);

	context_->GetSharedUniformBuffers()->UpdateUniformBuffer(SharedLightUniformBuffer::GetName(),& p, 0, sizeof(p));
}

void Rendering::OnPostRender() {

}

void Rendering::OnImageEffects(const std::vector<ImageEffect*>& effects) {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();
	RenderTexture* targets[] = {
		context_->GetOffscreenRenderTexture(),
		RenderTexture::GetTemporary(RenderTextureFormat::Rgba, w, h)
	};

	int index = 1;
	FrameState* frameState = context_->GetFrameState();
	for (int i = 0; i < effects.size(); ++i) {
		if (i + 1 == effects.size()) {
			targets[index] = frameState->targetTexture.get();
		}

		effects[i]->OnRenderImage(targets[1 - index], targets[index], frameState->normalizedRect);
		index = 1 - index;
	}

	RenderTexture::ReleaseTemporary(targets[1]);
}

void Rendering::SSAOPass(RenderingPipelines* pipelines) {
	ssaoSample->Restart();

	FrameState* fs = context_->GetFrameState();
	AmbientOcclusion* ao = context_->GetAmbientOcclusion();

	ao->Clear(fs->normalizedRect);
	ao->Run(context_->GetUniformState()->depthTexture.get(), fs->normalizedRect);

	ssaoSample->Stop();
	OutputSample(ssaoSample);
}

void Rendering::SSAOTraversalPass(RenderingPipelines* pipelines) {
	ssaoTraversalSample->Restart();
	pipelines->ssaoTraversal->Run();
	ssaoTraversalSample->Stop();
	OutputSample(ssaoTraversalSample);
}

void Rendering::DepthPass(RenderingPipelines* pipelines) {
	depthSample->Restart();
	if (pipelines->depth->GetRenderableCount() > 0) {
		pipelines->depth->Run();
	}
	depthSample->Stop();
	OutputSample(depthSample);
}

void Rendering::UpdateUniformBuffers(RenderingPipelines* pipelines, const RenderingMatrices& matrices) {
	UpdateTransformsUniformBuffer(matrices);

	FrameState* fs = context_->GetFrameState();
	if (fs->forwardBaseLight) {
		UpdateForwardBaseLightUniformBuffer(fs->forwardBaseLight);
	}
}

void Rendering::ShadowPass(RenderingPipelines* pipelines) {
	RenderTexture* target = pipelines->rendering->GetTargetTexture();

	ShadowMap* shadowMap = context_->GetShadowMap();
	shadowMap->Resize(target->GetWidth(), target->GetHeight());
	shadowMap->Clear();

	shadowSample->Restart();
	pipelines->shadow->Run();
	shadowSample->Stop();
	OutputSample(shadowSample);
}

void Rendering::RenderPass(RenderingPipelines* pipelines) {
	renderingSample->Restart();
	pipelines->rendering->Run();
	renderingSample->Stop();
	OutputSample(renderingSample);
}

PipelineBuilder::PipelineBuilder(Context* context) : context_(context) {
	forward_pass = Profiler::CreateSample();
	push_renderables = Profiler::CreateSample();
	get_renderable_game_objects = Profiler::CreateSample();
}

PipelineBuilder::~PipelineBuilder() {
	Profiler::ReleaseSample(forward_pass);
	Profiler::ReleaseSample(push_renderables);
	Profiler::ReleaseSample(get_renderable_game_objects);
}

void PipelineBuilder::Build(RenderingPipelines* pipelines, std::vector<GameObject*>& gameObjects, const RenderingMatrices& matrices) {
	matrices_ = matrices;
	pipelines->Clear();

	Matrix4 worldToClipMatrix = matrices_.projectionMatrix * matrices_.worldToCameraMatrix;

	ShadowMap* shadowMap = context_->GetShadowMap();
	for (int i = 0; i < gameObjects.size(); ++i) {
		GameObject* go = gameObjects[i];
		Material* material = shadowMap->GetMaterial();

		pipelines->shadow->AddRenderable(
			go->GetComponent<MeshProvider>()->GetMesh(), material, 0, go->GetTransform()->GetLocalToWorldMatrix()
		);
	}

	pipelines->shadow->Sort(SortModeMesh, worldToClipMatrix);

	bool depthPass = false;
	FrameState* fs = context_->GetFrameState();
	if (fs->renderPath == +RenderPath::Forward) {
		if ((fs->depthTextureMode & DepthTextureMode::Depth) != 0) {
			depthPass = true;
		}
	}

	if (Graphics::GetAmbientOcclusionEnabled()) {
		pipelines->ssaoTraversal->AssignRenderables(pipelines->shadow);
		SSAOPass(pipelines->ssaoTraversal);
		depthPass = true;
	}

	if (depthPass) {
		pipelines->depth->AssignRenderables(pipelines->shadow);
		ForwardDepthPass(pipelines->depth);
	}

	Light* forwardBase = nullptr;
	std::vector<Light*> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	RenderTexture* target = GetActiveRenderTarget();

	if (forwardBase) {
		shadowMap->Update(forwardBase);
	}

	pipelines->rendering->SetTargetTexture(target, fs->normalizedRect);
	if (fs->renderPath == +RenderPath::Forward) {
		ForwardRendering(pipelines->rendering, gameObjects, forwardBase, forwardAdd);
	}
	else {
		DeferredRendering(pipelines->rendering, gameObjects, forwardBase, forwardAdd);
	}

	pipelines->rendering->Sort(SortModeMeshMaterial, worldToClipMatrix);
}

void PipelineBuilder::ForwardRendering(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* forwardBase, const std::vector<Light*>& forwardAdd) {
	FrameState* fs = context_->GetFrameState();

	if (fs->clearType == +ClearType::Skybox) {
		RenderSkybox(pl);
	}

	fs->forwardBaseLight = forwardBase;
	if (forwardBase) {
		RenderForwardBase(pl, gameObjects, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(pl, gameObjects, forwardAdd);
	}

	RenderDecals(pl);
}

void PipelineBuilder::DeferredRendering(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* forwardBase, const std::vector<Light*>& forwardAdd) {
	// 	if (gbuffer_ == nullptr) {
	// 		InitializeDeferredRender();
	// 	}
	// 
	// 	RenderDeferredGeometryPass(target, gameObjects);
}

void PipelineBuilder::InitializeDeferredRender() {
	/*gbuffer_ = new GBuffer;
	gbuffer_->Create(Framebuffer0::Get()->GetViewportWidth(), Framebuffer0::Get()->GetViewportHeight());

	deferredMaterial_ = new Material();
	deferredMaterial_->SetRenderQueue(RenderQueueBackground);
	deferredMaterial_->SetShader(Shader::FindShader("builtin/gbuffer"));*/
}

void PipelineBuilder::RenderDeferredGeometryPass(Pipeline* pl, const std::vector<GameObject*>& gameObjects) {
}

void PipelineBuilder::RenderSkybox(Pipeline* pl) {
	Material* skybox = Environment::GetSkybox();
	if (skybox != nullptr) {
		Matrix4 matrix = matrices_.worldToCameraMatrix;
		matrix[3] = Vector4(0, 0, 0, 1);

		pl->AddRenderable(Mesh::GetPrimitive(PrimitiveType::Cube), skybox, 0, matrix);
	}
}

RenderTexture* PipelineBuilder::GetActiveRenderTarget() {
	FrameState* fs = context_->GetFrameState();
	if (!fs->camera->GetComponents<ImageEffect>().empty()) {
		return context_->GetOffscreenRenderTexture();
	}

	RenderTexture* target = fs->targetTexture.get();
	if (target == nullptr) {
		target = RenderTexture::GetDefault();
	}

	return target;
}

void PipelineBuilder::RenderForwardBase(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* light) {
	forward_pass->Restart();
	ForwardPass(pl, gameObjects);
	forward_pass->Stop();
	Debug::Output("[RenderableTraits::RenderForwardBase::forward_pass]\t%.2f", forward_pass->GetElapsedSeconds());
}

void PipelineBuilder::RenderForwardAdd(Pipeline* pl, const std::vector<GameObject*>& gameObjects, const std::vector<Light*>& lights) {
}

void PipelineBuilder::SSAOPass(Pipeline* pl) {
	ReplaceMaterials(pl, context_->GetAmbientOcclusion()->GetTraversalMaterial());
}

void PipelineBuilder::ForwardDepthPass(Pipeline* pl) {
	ReplaceMaterials(pl, context_->GetDepthMaterial());
}

void PipelineBuilder::ForwardPass(Pipeline* pl, const std::vector<GameObject*>& gameObjects) {
	for (int i = 0; i < gameObjects.size(); ++i) {
		GameObject* go = gameObjects[i];
		RenderGameObject(pl, go, go->GetComponent<Renderer>());
	}

	Debug::Output("[RenderableTraits::ForwardPass::push_renderables]\t%.2f", push_renderables->GetElapsedSeconds());
	push_renderables->Reset();
}

void PipelineBuilder::GetLights(Light*& forwardBase, std::vector<Light*>& forwardAdd) {
	std::vector<Light*> lights = World::GetComponents<Light>();
	if (lights.empty()) {
		return;
	}

	Light* first = lights.front();
	if (first->GetType() == LightType::Directional) {
		forwardBase = first;
	}

	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back((Light*)(lights[i]));
	}
}

void PipelineBuilder::RenderDecals(Pipeline* pl) {
	std::vector<Decal> decals;
	World::GetDecals(decals);

	for (Decal& d : decals) {
		pl->AddRenderable(d.mesh.get(), d.material.get(), 0, Matrix4(1));
	}
}

void PipelineBuilder::ReplaceMaterials(Pipeline* pl, Material* material) {
	uint nrenderables = pl->GetRenderableCount();
	for (uint i = 0; i < nrenderables; ++i) {
		Renderable& renderable = pl->GetRenderable(i);
		renderable.material = material;
		renderable.instance = 0;
	}
}

void PipelineBuilder::RenderGameObject(Pipeline* pl, GameObject* go, Renderer* renderer) {
	push_renderables->Start();

	int subMeshCount = go->GetComponent<MeshProvider>()->GetMesh()->GetSubMeshCount();
	int materialCount = renderer->GetMaterialCount();

	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	renderer->UpdateMaterialProperties();

	for (int i = 0; i < subMeshCount; ++i) {
		Material* material = renderer->GetMaterial(i);
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

void PipelineBuilder::RenderSubMesh(Pipeline* pl, GameObject* go, int subMeshIndex, Material* material, int pass) {
	ParticleSystem* p = go->GetComponent<ParticleSystem>();
	uint instance = p ? p->GetParticlesCount() : 0;

	pl->AddRenderable(go->GetComponent<MeshProvider>()->GetMesh(), subMeshIndex, material, pass, go->GetTransform()->GetLocalToWorldMatrix(), instance);
}
