#include <algorithm>
#include "pipeline.h"
#include "tools/math2.h"
#include "internal/world/ubomanager.h"

Camera Pipeline::camera_;
Pipeline* Pipeline::current_;
FramebufferBase* Pipeline::framebuffer_;

#define CLOCKS_PER_SEC_F	float(CLOCKS_PER_SEC)

#include <ctime>
static clock_t switch_framebuffer = 0, switch_material = 0, switch_mesh = 0, update_ubo = 0;
static clock_t set_buffer = 0;
static uint draw_calls = 0;

struct RenderableComparer {
	// TODO: hash renderable.
	bool operator () (Renderable& lhs, Renderable& rhs) const {
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

Pipeline::Pipeline() :renderables_(1024), nrenderables_(0)
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
	Debug::StartSample();

	Debug::StartSample();
	SortRenderables();
	Debug::Output("[sort]\t%.2f\n", Debug::EndSample());

	Debug::StartSample();
	uint i = 1, start = 0;
	glm::mat4 worldToClipMatrix = camera_->GetProjectionMatrix() * camera_->GetTransform()->GetWorldToLocalMatrix();
	for (; i < nrenderables_; ++i) {
		if (!renderables_[i].IsInstance(renderables_[start])) {
			RenderInstanced(start, i, worldToClipMatrix);
			start = i;
		}
	}

	RenderInstanced(start, nrenderables_, worldToClipMatrix);

	Debug::Output("[drawcall]\t%d\n", draw_calls);
	Debug::Output("[instanced]\t%.2f/%.2f\n", update_ubo / CLOCKS_PER_SEC_F, Debug::EndSample());
	
	Debug::Output("[fb]\t%.2f\n", switch_framebuffer / CLOCKS_PER_SEC_F);
	Debug::Output("[mat]\t%.2f\n", switch_material / CLOCKS_PER_SEC_F);
	Debug::Output("[mesh]\t%.2f\n", switch_mesh / CLOCKS_PER_SEC_F);
	Debug::Output("[setBuffer]\t%.2f\n", set_buffer / CLOCKS_PER_SEC_F);
	Debug::Output("[pipeline]\t%.2f\n", Debug::EndSample());

	ResetState();
}

void Pipeline::RenderInstanced(uint first, uint last, const glm::mat4& worldToClipMatrix) {
	Renderable& ref = renderables_[first];
	static int maxInstances = UBOManager::GetMaxBlockSize() / sizeof(EntityUBOStructs::EntityMatrices);
	int instanceCount = last - first;
	for (int j = 0; j < instanceCount; j += maxInstances) {
		int count = Math::Min(instanceCount - j, maxInstances);
		ref.instance = count;

		std::vector<glm::mat4> matrices;
		matrices.reserve(2 * count);
		for (int k = first; k < first + count; ++k) {
			matrices.push_back(renderables_[k].localToWorldMatrix);
			matrices.push_back(worldToClipMatrix * renderables_[k].localToWorldMatrix);
		}

		first += count;

		clock_t s = clock();
		UBOManager::UpdateSharedBuffer(SharedUBONames::EntityMatricesInstanced, &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		update_ubo += clock() - s;

		Render(ref);
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

	Renderable& ref = renderables_[nrenderables_++];
	ref.instance = instance;
	ref.mesh = mesh;
	ref.subMeshIndex = subMeshIndex;
	ref.material = material;
	ref.pass = pass;
	ref.state = state;
	ref.localToWorldMatrix = localToWorldMatrix;
}

void Pipeline::Render(Renderable& ref) {
	if (oldTarget_ == nullptr || *oldTarget_ != ref.state) {
		clock_t delta = clock();
		if (oldTarget_ != nullptr) {
			oldTarget_->Unbind();
		}

		oldTarget_ = &ref.state;

		ref.state.BindWrite();

		switch_framebuffer += (clock() - delta);
	}

	if (ref.material != oldMaterial_) {
		clock_t delta = clock();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = ref.pass;
		oldMaterial_ = ref.material;
		
		ref.material->Bind(ref.pass);
		switch_material += (clock() - delta);
	}
	else if (oldPass_ != ref.pass) {
		ref.material->Bind(ref.pass);
		oldPass_ = ref.pass;
	}

	if (ref.mesh->GetNativePointer() != oldMeshPointer_) {
		clock_t delta = clock();
		ref.mesh->Bind();
		oldMeshPointer_ = ref.mesh->GetNativePointer();
		switch_mesh += (clock() - delta);
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

void Pipeline::ResetState() {
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

	nrenderables_ = 0;
}
