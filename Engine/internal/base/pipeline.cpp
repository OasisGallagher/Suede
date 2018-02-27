#include <algorithm>
#include "pipeline.h"
#include "containers/freelist.h"

static free_list<Property> properties_(20480);
static free_list<Renderable> renderables_(20480);

struct RenderableComparer {
	bool operator () (Renderable* lhs, Renderable* rhs) const {
		Material& lm = lhs->material, &rm = rhs->material;
		if (lm->GetRenderQueue() != rm->GetRenderQueue()) {
			return lm->GetRenderQueue() < rm->GetRenderQueue();
		}

		uint lp = lm->GetPassNativePointer(lhs->pass);
		uint rp = lm->GetPassNativePointer(rhs->pass);
		if (lp != rp) {
			return lp < rp;
		}

		uint lme = lhs->subMesh->GetMesh()->GetNativePointer();
		uint rme = rhs->subMesh->GetMesh()->GetNativePointer();
		if (lme != rme) {
			return lme < rme;
		}

		return false;
	}
};

static RenderableComparer comparer;

void Pipeline::Update() {
	std::vector<Renderable*> container;
	container.reserve(renderables_.size());

	for (free_list<Renderable>::iterator ite = renderables_.begin(); ite != renderables_.end(); ++ite) {
		container.push_back(*ite);
	}

	std::sort(container.begin(), container.end(), comparer);

	for (std::vector<Renderable*>::iterator ite = container.begin(); ite != container.end(); ++ite) {
		Renderable* p = *ite;
		p->material->Bind(p->pass);

		Mesh mesh = p->subMesh->GetMesh();
		mesh->Bind();

		const TriangleBase& base = p->subMesh->GetTriangles();

		GLenum mode = TopologyToGLEnum(mesh->GetTopology());
		if (p->instance == 0) {
			GL::DrawElementsBaseVertex(mode, base.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* base.baseIndex), base.baseVertex);
		}
		else {
			GL::DrawElementsInstancedBaseVertex(mode, base.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* base.baseIndex), p->instance, base.baseVertex);
		}

		mesh->Unbind();
		p->material->Unbind();
	}

	properties_.clear();
	renderables_.clear();
}

GLenum Pipeline::TopologyToGLEnum(MeshTopology topology) {
	if (topology != MeshTopologyTriangles && topology != MeshTopologyTriangleStripes) {
		Debug::LogError("invalid mesh topology");
		return 0;
	}

	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

Property* Pipeline::CreateProperty() {
	return properties_.spawn();
}

Renderable* Pipeline::CreateRenderable() {
	Renderable* answer = renderables_.spawn();
	if (answer != nullptr) {
		ResetRenderable(answer);
	}

	return answer;
}

void Pipeline::ResetRenderable(Renderable* answer) {
	memset(answer->properties, 0, sizeof(answer->properties));
}
