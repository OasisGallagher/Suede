#include "mesh.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Mesh mesh = Resources::CreatePrimitive(PrimitiveTypeQuad, 2);

	material->SetRenderState(Cull, Off);
	material->SetRenderState(Blend, Off);
	material->SetRenderState(DepthWrite, Off);
	material->SetRenderState(DepthTest, Always);
	material->SetTexture(Variables::mainTexture, src);

	Resources::GetMeshRenderer()->SetMaterial(0, material);
	Resources::GetMeshRenderer()->RenderMesh(mesh, material);
}
