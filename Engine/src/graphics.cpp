#include "engine.h"

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	Mesh mesh = Resources::GetPrimitive(PrimitiveTypeQuad);
	Renderer renderer = New<MeshRenderer>();

	material->SetRenderState(Cull, Off);
	material->SetRenderState(DepthWrite, Off);
	material->SetRenderState(DepthTest, Always);
	material->SetTexture(Variables::mainTexture, src);

	renderer->RenderMesh(mesh, material);
}