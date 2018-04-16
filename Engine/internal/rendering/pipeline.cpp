#include <algorithm>

#include "time2.h"
#include "engine.h"
#include "pipeline.h"
#include "tools/math2.h"
#include "debug/profiler.h"
#include "internal/world/uniformbuffermanager.h"

Camera Pipeline::camera_;
Pipeline* Pipeline::current_;
FramebufferBase* Pipeline::framebuffer_;

struct RenderableComparer {
	// TODO: hash renderable.
	bool operator () (Renderable& lhs, Renderable& rhs) const {
		if (lhs.framebufferState.framebuffer != rhs.framebufferState.framebuffer) {
			return lhs.framebufferState.framebuffer < rhs.framebufferState.framebuffer;
		}

		Material& lm = lhs.material, &rm = rhs.material;
		if (lm->GetRenderQueue() != rm->GetRenderQueue()) {
			return lm->GetRenderQueue() < rm->GetRenderQueue();
		}

		if (lm != rm) {
			return lm < rm;
		}

		if (lhs.pass != rhs.pass) {
			return lhs.pass < rhs.pass;
		}

		uint lp = lm->GetPassNativePointer(lhs.pass);
		uint rp = lm->GetPassNativePointer(rhs.pass);
		if (lp != rp) {
			return lp < rp;
		}

		uint lme = lhs.mesh->GetNativePointer();
		uint rme = rhs.mesh->GetNativePointer();
		if (lme != rme) {
			return lme < rme;
		}

		if (lhs.subMeshIndex != rhs.subMeshIndex) {
			return lhs.subMeshIndex < rhs.subMeshIndex;
		}

		return false;
	}
};

Pipeline::Pipeline() :renderables_(1024), nrenderables_(0)
	, oldFramebufferState_(nullptr), oldPass_(-1) {
	switch_material = Profiler::CreateSample();
	switch_framebuffer = Profiler::CreateSample();
	switch_mesh = Profiler::CreateSample();
	update_ubo = Profiler::CreateSample();
	gather_instances = Profiler::CreateSample();
	update_pipeline = Profiler::CreateSample();
	sort_renderables = Profiler::CreateSample();
	rendering = Profiler::CreateSample();

	Engine::AddFrameEventListener(this);
}

Pipeline::~Pipeline() {
	Profiler::ReleaseSample(switch_material);
	Profiler::ReleaseSample(switch_framebuffer);
	Profiler::ReleaseSample(switch_mesh);
	Profiler::ReleaseSample(update_ubo);
	Profiler::ReleaseSample(gather_instances);
	Profiler::ReleaseSample(update_pipeline);
	Profiler::ReleaseSample(sort_renderables);
	Profiler::ReleaseSample(rendering);

	Engine::RemoveFrameEventListener(this);
}

GLenum TopologyToGLEnum(MeshTopology topology) {
	if (topology != MeshTopologyTriangles && topology != MeshTopologyTriangleStripes) {
		Debug::LogError("unsupported mesh topology  %d.", topology);
		return 0;
	}

	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

void Pipeline::SetFramebuffer(FramebufferBase* value) {
	if (value == nullptr) {
		value = Framebuffer0::Get();
	}

	framebuffer_ = value;
}

void Pipeline::OnFrameEnter() {

}

void Pipeline::OnFrameLeave() {

}

void Pipeline::Update() {
	update_pipeline->Restart();

	//sort_renderables->Restart();
	SortRenderables();
	//sort_renderables->Stop();
	//Debug::Output("[sort]\t%.2f\n", sort_renderables->GetElapsedSeconds());

	gather_instances->Restart();
	std::vector<uint> ranges;
	GatherInstances(ranges);
	gather_instances->Stop();
	Debug::Output("[Pipeline::Update::gather]\t%.2f\n", gather_instances->GetElapsedSeconds());

	//rendering->Restart();
	glm::mat4 worldToClipMatrix = camera_->GetProjectionMatrix() * camera_->GetTransform()->GetWorldToLocalMatrix();
	uint from = 0;
	for (std::vector<uint>::iterator ite = ranges.begin(); ite != ranges.end(); ++ite) {
		if (renderables_[from].instance != 0) {
			Debug::Break();
			Render(renderables_[from]);
		}
		else {
			RenderInstances(from, *ite, worldToClipMatrix);
		}

		from = *ite;
	}
	//rendering->Stop();

	//Debug::Output("[drawcall]\t%d\n", ndrawcalls);
	//Debug::Output("[update_ubo]\t%.2f\n", update_ubo->GetElapsedSeconds());
	//Debug::Output("[rendering]\t%.2f\n", rendering->GetElapsedSeconds());

	//Debug::Output("[fb]\t%.2f\n", switch_framebuffer->GetElapsedSeconds());
	//Debug::Output("[mat]\t%.2f\n", switch_material->GetElapsedSeconds());
	//Debug::Output("[mesh]\t%.2f\n", switch_mesh->GetElapsedSeconds());

	update_pipeline->Stop();
	Debug::Output("[Pipeline::Update::pipeline]\t%.2f\n", update_pipeline->GetElapsedSeconds());

	Clear();
}

void Pipeline::GatherInstances(std::vector<uint>& ranges) {
	uint base = 0;
	for (uint i = 0; i < nrenderables_; ++i) {
		// particle system.
		if (renderables_[i].instance != 0) {
			Debug::Break();

			// render instanced renderables.
			if (i > base) {
				ranges.push_back(i);
			}

			ranges.push_back(i + 1);
			base = i + 1;
		}
		else if (i != base && !renderables_[base].IsInstance(renderables_[i])) {
			ranges.push_back(i);
			base = i;
		}
	}

	if (base != nrenderables_) {
		ranges.push_back(nrenderables_);
	}
}

void Pipeline::RenderInstances(uint first, uint last, const glm::mat4& worldToClipMatrix) {
	Renderable& renderable = renderables_[first];
	static int maxInstances = UniformBufferManager::GetMaxBlockSize() / sizeof(EntityMatricesUniforms);
	int instanceCount = last - first;

	for (int j = 0; j < instanceCount; ) {
		int count = Math::Min(instanceCount - j, maxInstances);
		renderable.instance = count;

		std::vector<glm::mat4> matrices;
		matrices.reserve(2 * count);
		for (int k = j + first, max = j + first + count; k < max; ++k) {
			matrices.push_back(renderables_[k].localToWorldMatrix);
			matrices.push_back(worldToClipMatrix * renderables_[k].localToWorldMatrix);
		}

		update_ubo->Restart();
		UniformBufferManager::UpdateSharedBuffer(EntityMatricesUniforms::GetName(), &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		update_ubo->Stop();

		Render(renderable);
		j += count;
	}
}

void Pipeline::SortRenderables() {
	static RenderableComparer comparer;
	std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, comparer);
}

void Pipeline::AddRenderable(Mesh mesh, uint subMeshIndex, Material material, uint pass, const FramebufferState& state, const glm::mat4& localToWorldMatrix, uint instance) {
	if (nrenderables_ == renderables_.size()) {
		renderables_.resize(2 * nrenderables_);
	}

	Renderable& renderable = renderables_[nrenderables_++];
	renderable.instance = instance;
	renderable.mesh = mesh;
	renderable.subMeshIndex = subMeshIndex;
	renderable.material = material;
	renderable.pass = pass;
	renderable.framebufferState = state;
	renderable.localToWorldMatrix = localToWorldMatrix;
}

void Pipeline::Render(Renderable& renderable) {
	UpdateRenderContext(renderable);

	const TriangleBias& bias = renderable.mesh->GetSubMesh(renderable.subMeshIndex)->GetTriangleBias();

	GLenum mode = TopologyToGLEnum(renderable.mesh->GetTopology());
	if (renderable.instance == 0) {
		GL::DrawElementsBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
	}
	else {
		GL::DrawElementsInstancedBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), renderable.instance, bias.baseVertex);
	}

	++ndrawcalls;
}

void Pipeline::UpdateRenderContext(Renderable& renderable) {
	if (oldFramebufferState_ == nullptr || *oldFramebufferState_ != renderable.framebufferState) {
		switch_framebuffer->Start();
// 		if (oldFramebufferState_ != nullptr) {
// 			oldFramebufferState_->Unbind();
// 		}

		oldFramebufferState_ = &renderable.framebufferState;
		//renderable.framebufferState.BindWrite();

		switch_framebuffer->Stop();
	}

	if (renderable.material != oldMaterial_) {
		switch_material->Start();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = renderable.pass;
		oldMaterial_ = renderable.material;

		renderable.material->Bind(renderable.pass);
		switch_material->Stop();
	}
	else if (oldPass_ != renderable.pass) {
		renderable.material->Bind(renderable.pass);
		oldPass_ = renderable.pass;
	}

	if (!oldMesh_ || renderable.mesh->GetNativePointer() != oldMesh_->GetNativePointer()) {
		switch_mesh->Start();
		
		if (oldMesh_) {
			oldMesh_->Unbind();
		}

		oldMesh_ = renderable.mesh;

		renderable.mesh->Bind();
		switch_mesh->Stop();
	}
}

void Pipeline::Clear() {
	ndrawcalls = 0;
	ResetRenderContext();

	switch_mesh->Clear();
	switch_material->Clear();
	switch_framebuffer->Clear();

	for (uint i = 0; i < nrenderables_; ++i) {
		renderables_[i].Clear();
	}

	nrenderables_ = 0;
}

void Pipeline::ResetRenderContext() {
	if (oldFramebufferState_ != nullptr) {
		//oldFramebufferState_->Unbind();
		oldFramebufferState_ = nullptr;
	}

	if (oldMaterial_) {
		oldMaterial_->Unbind();
		oldMaterial_.reset();
	}

	if (oldMesh_) {
		oldMesh_->Unbind();
		oldMesh_.reset();
	}
}

void Renderable::Clear() {
	mesh.reset();
	material.reset();
	framebufferState.Clear();
}

bool Renderable::IsInstance(const Renderable& other) const {
#define CHECK_INSTANCE(expr)	if (expr != other.expr) { return false; } else (void)0

	CHECK_INSTANCE(mesh->GetNativePointer());
	CHECK_INSTANCE(subMeshIndex);
	CHECK_INSTANCE(framebufferState.framebuffer);
	CHECK_INSTANCE(material);
	CHECK_INSTANCE(pass);

#undef CHECK_INSTANCE

	return true;
}
