#include "mesh.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Mesh mesh = Resources::CreatePrimitive(PrimitiveTypeQuad, 2);

	material->SetTexture(Variables::mainTexture, src);
	Resources::GetAuxMeshRenderer()->RenderMesh(mesh, material);
}
