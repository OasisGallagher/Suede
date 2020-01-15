#include "renderingthread.h"

#include "engine.h"
#include "scene.h"
#include "graphics.h"
#include "profiler.h"
#include "projector.h"
#include "imageeffect.h"
#include "particlesystem.h"

#include "shadowmap.h"
#include "ambientocclusion.h"
#include "renderingcontext.h"
#include "shareduniformbuffers.h"

RenderingPipelines::RenderingPipelines(RenderingContext* context) {
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

RenderingThread::RenderingThread(RenderingContext* context) : context_(context) {
	profiler_ = context_->GetProfiler();
	graphics_ = context_->GetGraphics();
}

RenderingThread::~RenderingThread() {
	Stop();
}

bool RenderingThread::OnWork() {
	return true;
}

#define OutputSample(sample)	Debug::OutputToConsole("%s costs %.2f ms", #sample, sample->GetElapsedSeconds() * 1000)

void RenderingThread::Render(RenderingPipelines* pipelines, const RenderingMatrices& matrices) {
	context_->ClearFrame();
	UpdateUniformBuffers(pipelines, matrices);

	DepthPass(pipelines);

	if (graphics_->GetAmbientOcclusionEnabled()) {
		SSAOTraversalPass(pipelines);
		SSAOPass(pipelines);
	}

	FrameState* frameState = context_->GetFrameState();
	if (frameState->forwardBaseLight) {
		ShadowPass(pipelines);
	}

	RenderPass(pipelines);

	auto effects = frameState->camera->GetComponents<ImageEffect>();
	for (auto ite = effects.begin(); ite != effects.end();) {
		if ((*ite)->GetActiveAndEnabled()) {
			++ite;
		}
		else {
			ite = effects.erase(ite);
		}
	}

	if (!effects.empty()) { OnImageEffects(effects); }

	//graphics_->Blit(context_->GetUniformState()->depthTexture.get(), nullptr);
}

void RenderingThread::UpdateTransformsUniformBuffer(const RenderingMatrices& matrices) {
	static SharedTransformsUniformBuffer p;
	p.worldToClipMatrix = matrices.projectionMatrix * matrices.worldToCameraMatrix;
	p.worldToCameraMatrix = matrices.worldToCameraMatrix;
	p.cameraToClipMatrix = matrices.projectionMatrix;
	p.worldToShadowMatrix = context_->GetShadowMap()->GetWorldToShadowMatrix();

	p.projParams = matrices.projParams;
	p.cameraPos = Vector4(matrices.cameraPos.x, matrices.cameraPos.y, matrices.cameraPos.z, 1);
	p.screenParams = Vector4((float)Screen::GetWidth(), (float)Screen::GetHeight(), 0.f, 0.f);

	context_->GetUniformState()->uniformBuffers->UpdateUniformBuffer(SharedTransformsUniformBuffer::GetName(),& p, 0, sizeof(p));
}

void RenderingThread::UpdateForwardBaseLightUniformBuffer(Light* light) {
	static SharedLightUniformBuffer p;

	Environment* env = context_->GetScene()->GetEnvironment();
	memcpy(&p.fogParams.color, &env->fogColor, sizeof(p.fogParams.color));
	p.fogParams.density = env->fogDensity;

	memcpy(&p.ambientColor, &env->ambientColor, sizeof(p.ambientColor));

	Vector3 pos = light->GetTransform()->GetPosition();
	p.lightPos = Vector4(pos.x, pos.y, pos.z, 1);

	Vector3 dir = light->GetTransform()->GetRotation() * Vector3::forward;
	p.lightDir = Vector4(dir.x, dir.y, dir.z, 0);
	Color color = light->GetColor() * light->GetIntensity();
	p.lightColor = Vector4(color.r, color.g, color.b, 1);

	context_->GetUniformState()->uniformBuffers->UpdateUniformBuffer(SharedLightUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void RenderingThread::OnImageEffects(const std::vector<ImageEffect*>& effects) {
	RenderTexture* temporary;
	RenderTexture* targetTextures[] = {
		context_->GetOffscreenRenderTexture(),
		temporary = RenderTexture::GetTemporary(RenderTextureFormat::Rgba, Screen::GetWidth(), Screen::GetHeight())
	};

	int index = 1;
	FrameState* frameState = context_->GetFrameState();
	for (int i = 0; i < effects.size(); ++i) {
		if (i + 1 == effects.size()) {
			targetTextures[index] = frameState->targetTexture.get();
		}

		effects[i]->OnRenderImage(targetTextures[1 - index], targetTextures[index], frameState->normalizedRect);
		index = 1 - index;
	}

	RenderTexture::ReleaseTemporary(temporary);
}

void RenderingThread::SSAOPass(RenderingPipelines* pipelines) {
	Sample* sample = profiler_->CreateSample();
	sample->Start();
	FrameState* fs = context_->GetFrameState();
	AmbientOcclusion* ao = context_->GetAmbientOcclusion();

	ao->Clear(fs->normalizedRect);
	ao->Run(context_->GetUniformState()->depthTexture.get(), fs->normalizedRect);

	sample->Stop();
	OutputSample(sample);
	profiler_->ReleaseSample(sample);
}

void RenderingThread::SSAOTraversalPass(RenderingPipelines* pipelines) {
	Sample* sample = profiler_->CreateSample();
	sample->Start();
	pipelines->ssaoTraversal->Run();
	sample->Stop();
	OutputSample(sample);
	profiler_->ReleaseSample(sample);
}

void RenderingThread::DepthPass(RenderingPipelines* pipelines) {
	Sample* sample = profiler_->CreateSample();
	sample->Start();
	if (pipelines->depth->GetRenderableCount() > 0) {
		pipelines->depth->Run();
	}
	sample->Stop();
	OutputSample(sample);
	profiler_->ReleaseSample(sample);
}

void RenderingThread::UpdateUniformBuffers(RenderingPipelines* pipelines, const RenderingMatrices& matrices) {
	UpdateTransformsUniformBuffer(matrices);

	FrameState* fs = context_->GetFrameState();
	if (fs->forwardBaseLight) {
		UpdateForwardBaseLightUniformBuffer(fs->forwardBaseLight);
	}
}

void RenderingThread::ShadowPass(RenderingPipelines* pipelines) {
	RenderTexture* target = pipelines->rendering->GetTargetTexture();

	ShadowMap* shadowMap = context_->GetShadowMap();
	shadowMap->Resize(target->GetWidth(), target->GetHeight());
	shadowMap->Clear();

	Sample* sample = profiler_->CreateSample();
	sample->Start();
	pipelines->shadow->Run();
	sample->Stop();
	OutputSample(sample);
	profiler_->ReleaseSample(sample);
}

void RenderingThread::RenderPass(RenderingPipelines* pipelines) {
	Sample* sample = profiler_->CreateSample();
	sample->Start();
	pipelines->rendering->Run();
	sample->Stop();
	OutputSample(sample);
	profiler_->ReleaseSample(sample);
}

PipelineBuilder::PipelineBuilder(RenderingContext* context) : context_(context) {
	profiler_ = context_->GetProfiler();
	graphics_ = context_->GetGraphics();
}

void PipelineBuilder::Build(RenderingPipelines* pipelines, std::vector<GameObject*>& gameObjects, const RenderingMatrices& matrices) {
	matrices_ = matrices;

	Matrix4 worldToClipMatrix = matrices_.projectionMatrix * matrices_.worldToCameraMatrix;

	ShadowMap* shadowMap = context_->GetShadowMap();
	for (int i = 0; i < gameObjects.size(); ++i) {
		GameObject* go = gameObjects[i];
		Material* material = shadowMap->GetMaterial();

		pipelines->shadow->AddRenderable(
			go->GetComponent<MeshProvider>()->GetMesh(), material, go->GetTransform()->GetLocalToWorldMatrix()
		);
	}

	pipelines->shadow->Sort(SortMode::ByMesh, worldToClipMatrix);

	bool depthPass = false;
	FrameState* fs = context_->GetFrameState();
	if (fs->renderPath == RenderPath::Forward) {
		if ((fs->depthTextureMode & DepthTextureMode::Depth) != 0) {
			depthPass = true;
		}
	}

	if (graphics_->GetAmbientOcclusionEnabled()) {
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

	pipelines->rendering->Sort(SortMode::ByMaterialAndMesh, worldToClipMatrix);
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
	Environment* env = context_->GetScene()->GetEnvironment();
	if (env->skybox) {
		Matrix4 matrix = matrices_.worldToCameraMatrix;
		matrix[3] = Vector4(0, 0, 0, 1);

		ref_ptr<Mesh> mesh = Mesh::FromGeometry(Geometry::GetPrimitive(PrimitiveType::Cube));
		pl->AddRenderable(mesh.get(), env->skybox.get(), matrix);
	}
}

RenderTexture* PipelineBuilder::GetActiveRenderTarget() {
	FrameState* fs = context_->GetFrameState();
	for (ImageEffect* effect : fs->camera->GetComponents<ImageEffect>()) {
		if (effect->GetActiveAndEnabled()) {
			return context_->GetOffscreenRenderTexture();
		}
	}

	RenderTexture* target = fs->targetTexture.get();
	if (target == nullptr) {
		target = RenderTexture::GetDefault();
	}

	return target;
}

void PipelineBuilder::RenderForwardBase(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* light) {
	Sample* sample = profiler_->CreateSample();
	sample->Start();
	ForwardPass(pl, gameObjects);
	sample->Stop();
	Debug::OutputToConsole("[RenderableTraits::RenderForwardBase::forward_pass]\t%.2f", sample->GetElapsedSeconds());
	profiler_->ReleaseSample(sample);
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
	Sample* sample = profiler_->CreateSample();
	sample->Start();
	for (int i = 0; i < gameObjects.size(); ++i) {
		GameObject* go = gameObjects[i];
		RenderGameObject(pl, go, go->GetComponent<Renderer>());
	}

	sample->Stop();
	Debug::OutputToConsole("[RenderableTraits::ForwardPass::push_renderables]\t%.2f", sample->GetElapsedSeconds());
	profiler_->ReleaseSample(sample);
}

void PipelineBuilder::GetLights(Light*& forwardBase, std::vector<Light*>& forwardAdd) {
	std::vector<Light*> lights = context_->GetScene()->GetComponents<Light>();
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
	context_->GetScene()->GetDecals(decals);

	for (Decal& d : decals) {
		pl->AddRenderable(d.mesh.get(), d.material.get(), Matrix4(1));
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
	int subMeshCount = go->GetComponent<MeshProvider>()->GetMesh()->GetSubMeshCount();
	int materialCount = renderer->GetMaterialCount();

	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	renderer->UpdateMaterialProperties();

	for (int i = 0; i < subMeshCount; ++i) {
		RenderSubMesh(pl, go, i, renderer->GetMaterial(i));
	}
}

void PipelineBuilder::RenderSubMesh(Pipeline* pl, GameObject* go, int subMeshIndex, Material* material) {
	ParticleSystem* p = go->GetComponent<ParticleSystem>();
	uint instance = p ? p->GetParticlesCount() : 0;

	pl->AddRenderable(go->GetComponent<MeshProvider>()->GetMesh(), subMeshIndex, material, go->GetTransform()->GetLocalToWorldMatrix(), instance);
}
