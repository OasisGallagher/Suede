#include <algorithm>

#include "time2.h"
#include "pipeline.h"
#include "tools/math2.h"
#include "debug/profiler.h"
#include "internal/world/uniformbuffermanager.h"

Camera Pipeline::camera_;
Pipeline* Pipeline::current_;
FramebufferBase* Pipeline::framebuffer_;

static uint draw_calls = 0;
static uint64 switch_framebuffer = 0, switch_material = 0, switch_mesh = 0, update_ubo = 0;

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

void Pipeline::Update() {
	Profiler::StartSample();

	Profiler::StartSample();
	SortDrawables();
	Debug::Output("[sort]\t%.2f\n", Profiler::EndSample());

	Profiler::StartSample();

	Profiler::StartSample();
	RangeContainer container;
	GatherInstances(container);
	Debug::Output("[gather]\t%.2f\n", Profiler::EndSample());

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

	//	uint start = 0;
	// 	for (uint i = 0; i < ndrawables_; ++i) {
	// 		// particle system.
	// 		if (drawables_[i].instance != 0) {
	// 			// render instanced drawables.
	// 			RenderInstanced(start, i, worldToClipMatrix);
	// 
	// 			Render(drawables_[i]);
	// 			start = i + 1;
	// 		}
	// 		else if (i != 0 && !drawables_[i].IsInstance(drawables_[start])) {
	// 			RenderInstanced(start, i, worldToClipMatrix);
	// 			start = i;
	// 		}
	// 	}
	// 
	// 	if (start != ndrawables_) {
	// 		RenderInstanced(start, ndrawables_, worldToClipMatrix);
	// 	}

	Debug::Output("[drawcall]\t%d\n", draw_calls);
	Debug::Output("[instanced]\t%.2f/%.2f\n", Profiler::TimeStampToSeconds(update_ubo), Profiler::EndSample());

	Debug::Output("[fb]\t%.2f\n", Profiler::TimeStampToSeconds(switch_framebuffer));
	Debug::Output("[mat]\t%.2f\n", Profiler::TimeStampToSeconds(switch_material));
	Debug::Output("[mesh]\t%.2f\n", Profiler::TimeStampToSeconds(switch_mesh));
	Debug::Output("[pipeline]\t%.2f\n", Profiler::EndSample());

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

		uint64 s = Profiler::GetTimeStamp();
		UniformBufferManager::UpdateSharedBuffer(EntityMatricesUniforms::GetName(), &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		update_ubo += Profiler::GetTimeStamp() - s;

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

	++draw_calls;
}

void Pipeline::UpdateRenderContext(Drawable& drawable) {
	if (oldTarget_ == nullptr || *oldTarget_ != drawable.state) {
		uint64 timeStamp = Profiler::GetTimeStamp();
		if (oldTarget_ != nullptr) {
			oldTarget_->Unbind();
		}

		oldTarget_ = &drawable.state;

		drawable.state.BindWrite();

		switch_framebuffer += (Profiler::GetTimeStamp() - timeStamp);
	}

	if (drawable.material != oldMaterial_) {
		uint64 timeStamp = Profiler::GetTimeStamp();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = drawable.pass;
		oldMaterial_ = drawable.material;

		drawable.material->Bind(drawable.pass);
		switch_material += (Profiler::GetTimeStamp() - timeStamp);
	}
	else if (oldPass_ != drawable.pass) {
		drawable.material->Bind(drawable.pass);
		oldPass_ = drawable.pass;
	}

	if (drawable.mesh->GetNativePointer() != oldMeshPointer_) {
		uint64 timeStamp = Profiler::GetTimeStamp();
		drawable.mesh->Bind();
		oldMeshPointer_ = drawable.mesh->GetNativePointer();
		switch_mesh += (Profiler::GetTimeStamp() - timeStamp);
	}
}

void Pipeline::Clear() {
	draw_calls = 0;
	switch_framebuffer = switch_material = switch_mesh = update_ubo = 0;

	ResetRenderContext();

	for (uint i = 0; i < ndrawables_; ++i) {
		drawables_[i].Clear();
	}

	ndrawables_ = 0;
}

void Pipeline::ResetRenderContext() {
	if (oldTarget_ != nullptr) {
		oldTarget_->Unbind();
		oldTarget_ = nullptr;
	}

	if (oldMaterial_) {
		oldMaterial_->Unbind();
	}

	oldMaterial_ = nullptr;

	oldMeshPointer_ = 0;
}
