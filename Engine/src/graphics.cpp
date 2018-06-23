#include "mesh.h"
#include "world.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"
#include "api/glutils.h"
#include "tools/math2.h"

static void DrawSubMeshes(Mesh mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		GLUtils::DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(i)->GetTriangleBias());
	}
}

static Material GetBlitMaterial() {
	static Material material;
	if (!material) {
		Shader shader = NewShader();
		shader->Load("builtin/blit");

		material = NewMaterial();
		material->SetShader(shader);
	}

	return material;
}

static Mesh CreateBlitMesh(const Rect &srcRect) {
	MeshAttribute attribute = { MeshTopologyTriangleStripe };

	glm::vec3 vertices[] = {
		glm::vec3(-1.f, -1.f, 0.f),
		glm::vec3(1.f, -1.f, 0.f),
		glm::vec3(-1.f, 1.f, 0.f),
		glm::vec3(1.f, 1.f, 0.f),
	};
	attribute.positions.assign(vertices, vertices + CountOf(vertices));

	glm::vec2 texCoords[] = {
		srcRect.GetLeftBottom(),
		srcRect.GetRightBottom(),
		srcRect.GetLeftTop(),
		srcRect.GetRightTop(),
	};
	attribute.texCoords.assign(texCoords, texCoords + CountOf(texCoords));

	int indexes[] = { 0, 1, 2, 3 };
	attribute.indexes.assign(indexes, indexes + CountOf(indexes));
	
	Mesh mesh = NewMesh();
	mesh->SetAttribute(attribute);
	mesh->AddSubMesh(NewSubMesh());

	TriangleBias bias{ attribute.indexes.size() };
	mesh->GetSubMesh(0)->SetTriangleBias(bias);

	return mesh;
}

void Graphics::Blit(Texture src, RenderTexture dest) {
	Blit(src, dest, GetBlitMaterial(), Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void Graphics::Blit(Texture src, RenderTexture dest, const Rect& rect) {
	Blit(src, dest, GetBlitMaterial(), rect, rect);
}

void Graphics::Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect) {
	Blit(src, dest, GetBlitMaterial(), srcRect, destRect);
}

void Graphics::Blit(Texture src, RenderTexture dest, Material material) {
	Blit(src, dest, material, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void Graphics::Blit(Texture src, RenderTexture dest, Material material, const Rect& rect) {
	Blit(src, dest, material, rect, rect);
}

void Graphics::Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect) {
	if (!dest) { dest = WorldInstance()->GetScreenRenderTarget(); }

	dest->BindWrite(destRect);
	material->SetTexture(Variables::mainTexture, src);
	
	Mesh mesh = CreateBlitMesh(srcRect);
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
