#include "mesh.h"
#include "world.h"
#include "api/gl.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"

static void DrawSubMeshes(Mesh mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		GLUtil::DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(i)->GetTriangleBias());
	}
}

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Mesh mesh = Resources::CreatePrimitive(PrimitiveTypeQuad, 2);
	if (!dest) { dest = WorldInstance()->GetScreenRenderTarget(); }

	// TODO: VIEWPORT RECT.
	dest->BindWrite(glm::vec4(0, 0, 1, 1));
	material->SetTexture(Variables::mainTexture, src);
	Draw(mesh, material);
	dest->Unbind();
}

void Graphics::Draw(Mesh mesh, Material material) {
	mesh->Bind();
	int pass = material->GetPass();
	if (pass >= 0) {
		material->Bind(pass);
		DrawSubMeshes(mesh);
		material->Unbind();
	}
	else {
		for (pass = 0; pass < material->GetPassCount(); ++pass) {
			material->Bind(pass);
			DrawSubMeshes(mesh);
			material->Unbind();
		}
	}

	mesh->Unbind();
}
