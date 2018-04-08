#include "mesh.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"
#include "wrappers/gl.h"

// TODO: enum to GLenum or vice versa.
extern GLenum TopologyToGLEnum(MeshTopology topology);

static void DrawSubMeshes(Mesh mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		const TriangleBias& bias = subMesh->GetTriangleBias();

		GLenum mode = TopologyToGLEnum(mesh->GetTopology());
		GL::DrawElementsBaseVertex(mode, bias.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* bias.baseIndex), bias.baseVertex);
	}
}

void Graphics::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Mesh mesh = Resources::CreatePrimitive(PrimitiveTypeQuad, 2);
	material->SetTexture(Variables::mainTexture, src);
	Draw(mesh, material);
}

void Graphics::Draw(Mesh mesh, Material material) {
	mesh->Bind();
	int pass = material->GetPass();
	if (pass >= 0) {
		material->Bind(pass);
		DrawSubMeshes(mesh);
		material->Unbind();
		return;
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
