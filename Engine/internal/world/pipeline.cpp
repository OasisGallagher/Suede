#include <algorithm>
#include "pipeline.h"
#include "tools/math2.h"
#include "internal/base/framebuffer.h"

static uint nrenderables_;
static std::vector<Renderable> renderables_(1024);

static Material oldMaterial;
static uint oldMeshPointer = 0;
static FramebufferBase* oldFbo = nullptr;

#include <ctime>
static clock_t switchFramebuffer = 0, switchMaterial = 0, switchMesh = 0;

struct RenderableComparer {
	bool operator () (Renderable& lhs, Renderable& rhs) const {
		Material& lm = lhs.material, &rm = rhs.material;
		if (lm->GetRenderQueue() != rm->GetRenderQueue()) {
			return lm->GetRenderQueue() < rm->GetRenderQueue();
		}

		uint lp = lm->GetPassNativePointer(lhs.pass);
		uint rp = lm->GetPassNativePointer(rhs.pass);
		if (lp != rp) {
			return lp < rp;
		}

		uint lme = lhs.subMesh->GetMesh()->GetNativePointer();
		uint rme = rhs.subMesh->GetMesh()->GetNativePointer();
		if (lme != rme) {
			return lme < rme;
		}

		return false;
	}
};


void Pipeline::Update() {
	Debug::StartSample();

	SortRenderables();

	for (uint i = 0; i < nrenderables_; ++i) {
		Render(renderables_[i]);
	}

	Debug::Output("[fb]\t%.2f\n", float(switchFramebuffer) / CLOCKS_PER_SEC);
	Debug::Output("[mat]\t%.2f\n", float(switchMaterial) / CLOCKS_PER_SEC);
	Debug::Output("[mesh]\t%.2f\n", float(switchMesh) / CLOCKS_PER_SEC);
	Debug::Output("[pipeline]\t%.2f\n", Debug::EndSample());

	ResetState();
}

void Pipeline::SortRenderables() {
	static RenderableComparer comparer;
	std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, comparer);
}

GLenum Pipeline::TopologyToGLEnum(MeshTopology topology) {
	if (topology != MeshTopologyTriangles && topology != MeshTopologyTriangleStripes) {
		Debug::LogError("invalid mesh topology");
		return 0;
	}

	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

Renderable* Pipeline::CreateRenderable() {
	if (nrenderables_ == renderables_.size()) {
		renderables_.resize(2 * nrenderables_);
	}

	Renderable* answer = &renderables_[nrenderables_++];
	ResetRenderable(answer);
	return answer;
}

void Pipeline::ResetRenderable(Renderable* answer) {
	answer->framebuffer = nullptr;
}

void Pipeline::Render(Renderable& p) {
	if (oldFbo != p.framebuffer) {
		clock_t delta = clock();
		if (oldFbo != nullptr) {
			oldFbo->Unbind();
		}

		oldFbo = p.framebuffer;

		p.framebuffer->BindWrite();
		switchFramebuffer += (clock() - delta);
	}

	if (p.material != oldMaterial) {
		clock_t delta = clock();
		p.material->Bind(p.pass);
		oldMaterial = p.material;
		switchMaterial += (clock() - delta);
	}

	Mesh mesh = p.subMesh->GetMesh();
	if (mesh->GetNativePointer() != oldMeshPointer) {
		clock_t delta = clock();
		mesh->Bind();
		oldMeshPointer = mesh->GetNativePointer();
		switchMesh += (clock() - delta);
	}

	const TriangleBias& bias = p.subMesh->GetTriangles();

	GLenum mode = TopologyToGLEnum(mesh->GetTopology());
	if (p.instance == 0) {
		GL::DrawElementsBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
	}
	else {
		GL::DrawElementsInstancedBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), p.instance, bias.baseVertex);
	}

	//mesh->Unbind();
	//p.material->Unbind();
}

void Pipeline::ResetState() {
	switchFramebuffer = switchMaterial = switchMesh = 0;

	if (oldFbo != nullptr) {
		oldFbo->Unbind();
		oldFbo = nullptr;
	}
	
	oldMeshPointer = 0;
	oldMaterial = nullptr;

	nrenderables_ = 0;
}
