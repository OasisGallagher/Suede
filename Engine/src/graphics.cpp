#include "mesh.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Mesh mesh = Resources::CreatePrimitive(PrimitiveTypeQuad, 2);
	static Renderer renderer = nullptr;
	
	if (!renderer) {
		renderer = NewMeshRenderer();
		renderer->AddMaterial(material);
	}

	material->SetRenderQueue(RenderQueueOverlay + 5000);
	material->SetTexture(Variables::mainTexture, src);
	renderer->RenderMesh(mesh);
}
