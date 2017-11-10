#include "variables.h"
#include "tools/math2.h"
#include "graphicsinternal.h"
#include "internal/memory/memory.h"
#include "internal/world/worldinternal.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/rendererinternal.h"

Graphics graphicsInstance(Memory::Create<GraphicsInternal>());

void GraphicsInternal::Blit(RenderTexture src, RenderTexture dest, Material material) {
	Surface surface = GetQuadSurface();
	Renderer renderer = CREATE_OBJECT(SurfaceRenderer);

	material->SetRenderState(Cull, Off);
	material->SetRenderState(DepthWrite, Off);
	material->SetRenderState(DepthTest, Always);
	material->SetTexture(Variables::mainTexture, src);

	material->Bind();
	renderer->RenderSurface(surface);
	material->Unbind();
}

Surface GraphicsInternal::GetQuadSurface() {
	static Surface surface;
	if (surface) {
		return surface;
	}

	glm::vec3 kQuadVertices[] = {
		glm::vec3(-1.f, -1.f, 0.f),
		glm::vec3(1.f, -1.f, 0.f),
		glm::vec3(-1.f,  1.f, 0.f),
		glm::vec3(1.f,  1.f, 0.f),
	};

	glm::vec2 kQuadTexCoords[] = {
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
	};

	const int kQuadIndexes[] = { 0, 1, 2, 3 };

	surface = CREATE_OBJECT(Surface);
	SurfaceAttribute attribute;
	attribute.indexes.assign(kQuadIndexes, kQuadIndexes + CountOf(kQuadIndexes));
	attribute.positions.assign(kQuadVertices, kQuadVertices + CountOf(kQuadVertices));
	attribute.texCoords.assign(kQuadTexCoords, kQuadTexCoords + CountOf(kQuadTexCoords));

	surface->SetAttribute(attribute);

	Mesh mesh = CREATE_OBJECT(Mesh);
	mesh->SetTopology(MeshTopologyTriangleStripes);
	mesh->SetTriangles(CountOf(kQuadVertices), 0, 0);

	surface->AddMesh(mesh);

	return surface;
}
