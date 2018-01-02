#include "mesh.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Mesh mesh = Resources::CreatePrimitive(PrimitiveTypeQuad, 2);

	material->SetTexture(Variables::mainTexture, src);

	Resources::GetMeshRenderer()->SetMaterial(0, material);
	Resources::GetMeshRenderer()->RenderMesh(mesh, material);
}
