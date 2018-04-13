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

struct DrawableComparer {
	// TODO: hash drawable.
	bool operator () (Drawable& lhs, Drawable& rhs) const {
		if (lhs.state.framebuffer != rhs.state.framebuffer) {
			return lhs.state.framebuffer < rhs.state.framebuffer;
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

Pipeline::Pipeline() :drawables_(1024), ndrawables_(0)
	, oldFramebuffer_(nullptr), oldPass_(-1), oldMeshPointer_(0) {
	switch_material = Profiler::CreateSample();
	switch_framebuffer = Profiler::CreateSample();
	switch_mesh = Profiler::CreateSample();
	update_ubo = Profiler::CreateSample();
	gather_instances = Profiler::CreateSample();
	update_pipeline = Profiler::CreateSample();
	sort_drawables = Profiler::CreateSample();
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
	Profiler::ReleaseSample(sort_drawables);
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

	//sort_drawables->Restart();
	SortDrawables();
	//sort_drawables->Stop();
	//Debug::Output("[sort]\t%.2f\n", sort_drawables->GetElapsedSeconds());

	gather_instances->Restart();
	RangeContainer container;
	GatherInstances(container);
	gather_instances->Stop();
	Debug::Output("[Pipeline::Update::gather]\t%.2f\n", gather_instances->GetElapsedSeconds());

	//rendering->Restart();
	glm::mat4 worldToClipMatrix = camera_->GetProjectionMatrix() * camera_->GetTransform()->GetWorldToLocalMatrix();
	for (RangeContainer::iterator ite = container.begin(); ite != container.end(); ++ite) {
		Range& r = *ite;
		if (drawables_[r.first].instance != 0) {
			Render(drawables_[r.first]);
		}
		else {
			RenderInstances(r.first, r.second, worldToClipMatrix);
		}
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

void Pipeline::GatherInstances(RangeContainer& container) {
	uint start = 0;
	for (uint i = 0; i < ndrawables_; ++i) {
		// particle system.
		if (drawables_[i].instance != 0) {
			// render instanced drawables.
			if (i > start) {
				container.push_back(Range(start, i));
			}

			container.push_back(Range(i, i + 1));
			start = i + 1;
		}
		else if (i != 0 && !drawables_[i].IsInstance(drawables_[start])) {
			container.push_back(Range(start, i));
			start = i;
		}
	}

	if (start != ndrawables_) {
		container.push_back(Range(start, ndrawables_));
	}
}

void Pipeline::RenderInstances(uint first, uint last, const glm::mat4& worldToClipMatrix) {
	Drawable& drawable = drawables_[first];
	static int maxInstances = UniformBufferManager::GetMaxBlockSize() / sizeof(EntityMatricesUniforms);
	int instanceCount = last - first;
	for (int j = 0; j < instanceCount; j += maxInstances) {
		int count = Math::Min(instanceCount - j, maxInstances);
		drawable.instance = count;

		std::vector<glm::mat4> matrices;
		matrices.reserve(2 * count);
		for (int k = first; k < first + count; ++k) {
			matrices.push_back(drawables_[k].localToWorldMatrix);
			matrices.push_back(worldToClipMatrix * drawables_[k].localToWorldMatrix);
		}

		first += count;

		update_ubo->Restart();
		UniformBufferManager::UpdateSharedBuffer(EntityMatricesUniforms::GetName(), &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		update_ubo->Stop();

		Render(drawable);
	}
}

void Pipeline::SortDrawables() {
	static DrawableComparer comparer;
	std::sort(drawables_.begin(), drawables_.begin() + ndrawables_, comparer);
}

void Pipeline::AddDrawable(Mesh mesh, uint subMeshIndex, Material material, uint pass, const FramebufferState& state, const glm::mat4& localToWorldMatrix, uint instance) {
	if (ndrawables_ == drawables_.size()) {
		drawables_.resize(2 * ndrawables_);
	}

	Drawable& drawable = drawables_[ndrawables_++];
	drawable.instance = instance;
	drawable.mesh = mesh;
	drawable.subMeshIndex = subMeshIndex;
	drawable.material = material;
	drawable.pass = pass;
	drawable.state = state;
	drawable.localToWorldMatrix = localToWorldMatrix;
}

void Pipeline::Render(Drawable& drawable) {
	UpdateRenderContext(drawable);

	const TriangleBias& bias = drawable.mesh->GetSubMesh(drawable.subMeshIndex)->GetTriangleBias();

	GLenum mode = TopologyToGLEnum(drawable.mesh->GetTopology());
	if (drawable.instance == 0) {
		GL::DrawElementsBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
	}
	else {
		GL::DrawElementsInstancedBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), drawable.instance, bias.baseVertex);
	}

	++ndrawcalls;
}

void Pipeline::UpdateRenderContext(Drawable& drawable) {
	if (oldFramebuffer_ == nullptr || *oldFramebuffer_ != drawable.state) {
		switch_framebuffer->Start();
		if (oldFramebuffer_ != nullptr) {
			oldFramebuffer_->Unbind();
		}

		oldFramebuffer_ = &drawable.state;
		drawable.state.BindWrite();

		switch_framebuffer->Stop();
	}

	if (drawable.material != oldMaterial_) {
		switch_material->Start();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = drawable.pass;
		oldMaterial_ = drawable.material;

		drawable.material->Bind(drawable.pass);
		switch_material->Stop();
	}
	else if (oldPass_ != drawable.pass) {
		drawable.material->Bind(drawable.pass);
		oldPass_ = drawable.pass;
	}

	if (drawable.mesh->GetNativePointer() != oldMeshPointer_) {
		switch_mesh->Start();
		drawable.mesh->Bind();
		oldMeshPointer_ = drawable.mesh->GetNativePointer();
		switch_mesh->Stop();
	}
}

void Pipeline::Clear() {
	ndrawcalls = 0;
	ResetRenderContext();

	switch_mesh->Clear();
	switch_material->Clear();
	switch_framebuffer->Clear();

	for (uint i = 0; i < ndrawables_; ++i) {
		drawables_[i].Clear();
	}

	ndrawables_ = 0;
}

void Pipeline::ResetRenderContext() {
	if (oldFramebuffer_ != nullptr) {
		oldFramebuffer_->Unbind();
		oldFramebuffer_ = nullptr;
	}

	if (oldMaterial_) {
		oldMaterial_->Unbind();
	}

	oldMaterial_ = nullptr;

	oldMeshPointer_ = 0;
}
