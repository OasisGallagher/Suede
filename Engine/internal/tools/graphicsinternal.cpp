#include "graphicsinternal.h"
#include "mesh.h"
#include "world.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "../api/glutils.h"

GraphicsInternal::GraphicsInternal() : mode_(ShadingMode::Shaded) {
	material_ = CreateBlitMaterial();
}

void GraphicsInternal::SetShadingMode(ShadingMode value) {
	if (mode_ != value) {
		GL::PolygonMode(GL_FRONT_AND_BACK, value == ShadingMode::Shaded ? GL_FILL : GL_LINE);
		mode_ = value;
	}
}

void GraphicsInternal::Blit(Texture src, RenderTexture dest) {
	Blit(src, dest, material_, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void GraphicsInternal::Blit(Texture src, RenderTexture dest, const Rect& rect) {
	Blit(src, dest, material_, rect, rect);
}

void GraphicsInternal::Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect) {
	Blit(src, dest, material_, srcRect, destRect);
}

void GraphicsInternal::Blit(Texture src, RenderTexture dest, Material material) {
	Blit(src, dest, material, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void GraphicsInternal::Blit(Texture src, RenderTexture dest, Material material, const Rect& rect) {
	Blit(src, dest, material, rect, rect);
}

void GraphicsInternal::Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect) {
	if (!dest) { dest = RenderTexture::GetDefault(); }

	dest->BindWrite(destRect);
	material->SetTexture(Variables::MainTexture, src);

	Mesh mesh = CreateBlitMesh(srcRect);
	Draw(mesh, material);

	dest->Unbind();
}

void GraphicsInternal::Draw(Mesh mesh, Material material) {
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

Mesh GraphicsInternal::CreateBlitMesh(const Rect& rect) {
	MeshAttribute attribute = { MeshTopology::TriangleStripe };

	glm::vec3 vertices[] = {
		glm::vec3(-1.f, -1.f, 0.f),
		glm::vec3(1.f, -1.f, 0.f),
		glm::vec3(-1.f, 1.f, 0.f),
		glm::vec3(1.f, 1.f, 0.f),
	};
	attribute.positions.assign(vertices, vertices + CountOf(vertices));

	glm::vec2 texCoords[] = {
		rect.GetLeftBottom(),
		rect.GetRightBottom(),
		rect.GetLeftTop(),
		rect.GetRightTop(),
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

void GraphicsInternal::DrawSubMeshes(Mesh mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		GLUtils::DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(i)->GetTriangleBias());
	}
}

Material GraphicsInternal::CreateBlitMaterial() {
	Material material = NewMaterial();
	Shader shader = NewShader();
	shader->Load("builtin/blit");

	material = NewMaterial();
	material->SetShader(shader);

	return material;
}
