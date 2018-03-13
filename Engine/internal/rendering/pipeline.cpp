#include <algorithm>
#include "pipeline.h"
#include "tools/math2.h"
#include "internal/world/ubomanager.h"
#include "internal/base/framebuffer.h"

Camera Pipeline::camera_;
Pipeline* Pipeline::current_;
FramebufferBase* Pipeline::framebuffer_;

#include <ctime>
static clock_t switchFramebuffer = 0, switchMaterial = 0, switchMesh = 0, updateUBO = 0;
static clock_t setBuffer = 0;
static uint drawCall = 0;

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

void Pipeline::SetFramebuffer(FramebufferBase * value) {
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
	Debug::Output("[instanced]\t%d\t%.2f/%.2f\n", drawCall, float(updateUBO) / CLOCKS_PER_SEC, Debug::EndSample());

	RenderInstanced(start, nrenderables_, worldToClipMatrix);
	
	Debug::Output("[fb]\t%.2f\n", float(switchFramebuffer) / CLOCKS_PER_SEC);
	Debug::Output("[mat]\t%.2f\n", float(switchMaterial) / CLOCKS_PER_SEC);
	Debug::Output("[mesh]\t%.2f\n", float(switchMesh) / CLOCKS_PER_SEC);
	Debug::Output("[setBuffer]\t%.2f\n", float(setBuffer) / CLOCKS_PER_SEC);
	Debug::Output("[pipeline]\t%.2f\n", Debug::EndSample());

	ResetState();
}

void Pipeline::RenderInstanced(uint first, uint last, const glm::mat4& worldToClipMatrix) {
	Renderable& ref = renderables_[first];
	static int maxInstances = UBOManager::GetMaxBlockSize() / (2 * sizeof(glm::mat4));
	int instanceCount = last - first;
	for (int j = 0; j < instanceCount; j += maxInstances) {
		int count = Math::Min(instanceCount - j, maxInstances);
		ref.instance = count;

		std::vector<glm::mat4> matrices;
		matrices.reserve(2 * count);
		for (int k = first; k < first + count; ++k) {
			matrices.push_back(ref.localToWorldMatrix);
			matrices.push_back(worldToClipMatrix * ref.localToWorldMatrix);
		}

		clock_t s = clock();
		UBOManager::UpdateSharedBuffer(SharedUBONames::EntityMatricesInstanced, &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		updateUBO += clock() - s;

		Render(ref);
	}
}

void Pipeline::SortRenderables() {
	static RenderableComparer comparer;
	std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, comparer);
}

Renderable* Pipeline::BeginRenderable() {
	if (nrenderables_ == renderables_.size()) {
		renderables_.resize(2 * nrenderables_);
	}

	Renderable* answer = &renderables_[nrenderables_++];
	ClearRenderable(answer);
	return answer;
}

void Pipeline::EndRenderable() {

}

void Pipeline::ClearRenderable(Renderable* answer) {
	answer->state.Clear();
}

void Pipeline::Render(Renderable& p) {
	if (oldTarget_ == nullptr || *oldTarget_ != p.state) {
		clock_t delta = clock();
		if (oldTarget_ != nullptr) {
			oldTarget_->Unbind();
		}

		oldTarget_ = &p.state;

		p.state.BindWrite();

		switchFramebuffer += (clock() - delta);
	}

	if (p.material != oldMaterial_) {
		clock_t delta = clock();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = p.pass;
		oldMaterial_ = p.material;
		
		p.material->Bind(p.pass);
		switchMaterial += (clock() - delta);
	}
	else if (oldPass_ != p.pass) {
		p.material->Bind(p.pass);
		oldPass_ = p.pass;
	}

// 	clock_t sss = clock();
// 	const size_t structureSize = sizeof(EntityUBOStructs::EntityMatrices);
// 	UBOManager::AttachEntityBuffer(p.material->GetShader(), p.mesh->GetSubMesh(p.subMeshIndex)->__GetIndex());
// 	setBuffer += clock() - sss;

	if (p.mesh->GetNativePointer() != oldMeshPointer_) {
		clock_t delta = clock();
		p.mesh->Bind();
		oldMeshPointer_ = p.mesh->GetNativePointer();
		switchMesh += (clock() - delta);
	}

	const TriangleBias& bias = p.mesh->GetSubMesh(p.subMeshIndex)->GetTriangleBias();

	GLenum mode = TopologyToGLEnum(p.mesh->GetTopology());
	if (p.instance == 0) {
		GL::DrawElementsBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
	}
	else {
		GL::DrawElementsInstancedBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), p.instance, bias.baseVertex);
	}

	++drawCall;
}

void Pipeline::ResetState() {
	drawCall = 0;
	switchFramebuffer = switchMaterial = switchMesh = setBuffer = updateUBO = 0;

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
