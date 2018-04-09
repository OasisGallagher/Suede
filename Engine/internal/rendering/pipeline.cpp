#include <algorithm>

#include "time2.h"
#include "pipeline.h"
#include "tools/math2.h"
#include "debug/profiler.h"
#include "internal/world/uniformbuffermanager.h"

Camera Pipeline::camera_;
Pipeline* Pipeline::current_;
FramebufferBase* Pipeline::framebuffer_;

static uint64 switch_framebuffer = 0, switch_material = 0, switch_mesh = 0, update_ubo = 0;
static uint64 set_buffer = 0;
static uint draw_calls = 0;

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
	, oldTarget_(nullptr), oldPass_(-1), oldMeshPointer_(0) {
}

GLenum TopologyToGLEnum(MeshTopology topology) {
	if (topology != MeshTopologyTriangles && topology != MeshTopologyTriangleStripes) {
		Debug::LogError("invalid mesh topology");
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

void Pipeline::Update() {
	Profiler::StartSample();

	Profiler::StartSample();
	SortDrawables();
	Debug::Output("[sort]\t%.2f\n", Profiler::EndSample());

	Profiler::StartSample();

	uint start = 0;
	glm::mat4 worldToClipMatrix = camera_->GetProjectionMatrix() * camera_->GetTransform()->GetWorldToLocalMatrix();
	for (uint i = 0; i < ndrawables_; ++i) {
		if (drawables_[i].instance != 0) {
			RenderInstanced(start, i, worldToClipMatrix);

			// render particle system.
			Render(drawables_[i]);
			start = i + 1;
		}
		else if (i != 0 && !drawables_[i].IsInstance(drawables_[start])) {
			RenderInstanced(start, i, worldToClipMatrix);
			start = i;
		}
	}

	if (start != ndrawables_) {
		RenderInstanced(start, ndrawables_, worldToClipMatrix);
	}

	Debug::Output("[drawcall]\t%d\n", draw_calls);
	Debug::Output("[instanced]\t%.2f/%.2f\n", update_ubo * Profiler::GetSecondsPerTick(), Profiler::EndSample());
	
	Debug::Output("[fb]\t%.2f\n", switch_framebuffer * Profiler::GetSecondsPerTick());
	Debug::Output("[mat]\t%.2f\n", switch_material * Profiler::GetSecondsPerTick());
	Debug::Output("[mesh]\t%.2f\n", switch_mesh * Profiler::GetSecondsPerTick());
	Debug::Output("[setBuffer]\t%.2f\n", set_buffer * Profiler::GetSecondsPerTick());
	Debug::Output("[pipeline]\t%.2f\n", Profiler::EndSample());

	Clear();
}

void Pipeline::RenderInstanced(uint first, uint last, const glm::mat4& worldToClipMatrix) {
	Drawable& ref = drawables_[first];
	static int maxInstances = UniformBufferManager::GetMaxBlockSize() / sizeof(EntityUBOStructs::EntityMatrices);
	int instanceCount = last - first;
	for (int j = 0; j < instanceCount; j += maxInstances) {
		int count = Math::Min(instanceCount - j, maxInstances);
		ref.instance = count;

		std::vector<glm::mat4> matrices;
		matrices.reserve(2 * count);
		for (int k = first; k < first + count; ++k) {
			matrices.push_back(drawables_[k].localToWorldMatrix);
			matrices.push_back(worldToClipMatrix * drawables_[k].localToWorldMatrix);
		}

		first += count;

		uint64 s = Profiler::GetTicks();
		UniformBufferManager::UpdateSharedBuffer(SharedUBONames::EntityMatricesInstanced, &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		update_ubo += Profiler::GetTicks() - s;

		Render(ref);
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

	Drawable& ref = drawables_[ndrawables_++];
	ref.instance = instance;
	ref.mesh = mesh;
	ref.subMeshIndex = subMeshIndex;
	ref.material = material;
	ref.pass = pass;
	ref.state = state;
	ref.localToWorldMatrix = localToWorldMatrix;
}

void Pipeline::Render(Drawable& ref) {
	if (oldTarget_ == nullptr || *oldTarget_ != ref.state) {
		uint64 delta = Profiler::GetTicks();
		if (oldTarget_ != nullptr) {
			oldTarget_->Unbind();
		}

		oldTarget_ = &ref.state;

		ref.state.BindWrite();

		switch_framebuffer += (Profiler::GetTicks() - delta);
	}

	if (ref.material != oldMaterial_) {
		uint64 delta = Profiler::GetTicks();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = ref.pass;
		oldMaterial_ = ref.material;
		
		ref.material->Bind(ref.pass);
		switch_material += (Profiler::GetTicks() - delta);
	}
	else if (oldPass_ != ref.pass) {
		ref.material->Bind(ref.pass);
		oldPass_ = ref.pass;
	}

	if (ref.mesh->GetNativePointer() != oldMeshPointer_) {
		uint64 delta = Profiler::GetTicks();
		ref.mesh->Bind();
		oldMeshPointer_ = ref.mesh->GetNativePointer();
		switch_mesh += (Profiler::GetTicks() - delta);
	}

	const TriangleBias& bias = ref.mesh->GetSubMesh(ref.subMeshIndex)->GetTriangleBias();

	GLenum mode = TopologyToGLEnum(ref.mesh->GetTopology());
	if (ref.instance == 0) {
		GL::DrawElementsBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
	}
	else {
		GL::DrawElementsInstancedBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), ref.instance, bias.baseVertex);
	}

	++draw_calls;
}

void Pipeline::Clear() {
	draw_calls = 0;
	switch_framebuffer = switch_material = switch_mesh = set_buffer = update_ubo = 0;

	if (oldTarget_ != nullptr) {
		oldTarget_->Unbind();
		oldTarget_ = nullptr;
	}
	
	if (oldMaterial_) {
		oldMaterial_->Unbind();
	}

	oldMaterial_ = nullptr;

	oldMeshPointer_ = 0;

	for (uint i = 0; i < ndrawables_; ++i) {
		drawables_[i].Clear();
	}

	ndrawables_ = 0;
}
