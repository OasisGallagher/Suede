#include "graphicsinternal.h"
#include "mesh.h"
#include "world.h"
#include "graphics.h"
#include "resources.h"
#include "tools/math2.h"
#include "../api/glutils.h"

#include "memory/memory.h"
#include "builtinproperties.h"

Graphics::Graphics() : singleton2<Graphics>(MEMORY_NEW(GraphicsInternal), Memory::DeleteRaw<GraphicsInternal>) {}

void Graphics::SetShadingMode(ShadingMode value) { _suede_dinstance()->SetShadingMode(value); }
ShadingMode Graphics::GetShadingMode() { return _suede_dinstance()->GetShadingMode(); }
void Graphics::SetAmbientOcclusionEnabled(bool value) { _suede_dinstance()->SetAmbientOcclusionEnabled(value); }
bool Graphics::GetAmbientOcclusionEnabled() { return _suede_dinstance()->GetAmbientOcclusionEnabled(); }
void Graphics::SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer) { _suede_dinstance()->SetRenderTarget(colorBuffers, depthBuffer); }
void Graphics::Draw(Mesh mesh, Material material) { _suede_dinstance()->Draw(mesh, material); }
void Graphics::Draw(Mesh mesh, Material* materials, uint materialCount) { _suede_dinstance()->Draw(mesh, materials, materialCount); }
void Graphics::Blit(Texture src, RenderTexture dest) { _suede_dinstance()->Blit(src, dest); }
void Graphics::Blit(Texture src, RenderTexture dest, const Rect& rect) { _suede_dinstance()->Blit(src, dest, rect); }
void Graphics::Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect) { _suede_dinstance()->Blit(src, dest, srcRect, destRect); }
void Graphics::Blit(Texture src, RenderTexture dest, Material material) { _suede_dinstance()->Blit(src, dest, material); }
void Graphics::Blit(Texture src, RenderTexture dest, Material material, const Rect& rect) { _suede_dinstance()->Blit(src, dest, material, rect); }
void Graphics::Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect) { _suede_dinstance()->Blit(src, dest, material, srcRect, destRect); }

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
	if (!dest) { dest = RenderTextureUtility::GetDefault(); }

	dest->BindWrite(destRect);
	material->SetTexture(BuiltinProperties::MainTexture, src);

	Mesh mesh = CreateBlitMesh(srcRect);
	Draw(mesh, material);

	dest->Unbind();
}

void GraphicsInternal::SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer) {
}

void GraphicsInternal::Draw(Mesh mesh, Material material) {
	mesh->Bind();
	int pass = material->GetPass();
	if (pass >= 0) {
		DrawSubMeshesIfPassEnabled(mesh, material, pass);
	}
	else {
		// for each pass.
		for (pass = 0; pass < material->GetPassCount(); ++pass) {
			DrawSubMeshesIfPassEnabled(mesh, material, pass);
		}
	}

	mesh->Unbind();
}

void GraphicsInternal::Draw(Mesh mesh, Material* materials, uint materialCount) {
	if (mesh->GetSubMeshCount() != materialCount) {
		return Debug::LogError("material count mismatch with sub mesh count");
	}

	mesh->Bind();
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		Material material = materials[i];
		int pass = material->GetPass();
		if (pass >= 0) {
			DrawSubMeshIfPassEnabled(mesh, i, material, pass);
		}
		else {
			// for each pass.
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				DrawSubMeshIfPassEnabled(mesh, i, material, pass);
			}
		}
	}

	mesh->Unbind();
}

Mesh GraphicsInternal::CreateBlitMesh(const Rect& rect) {
	MeshAttribute attribute;
	attribute.topology = MeshTopology::TriangleStripe;

	attribute.positions.assign({
		glm::vec3(-1.f, -1.f, 0.f),
		glm::vec3(1.f, -1.f, 0.f),
		glm::vec3(-1.f, 1.f, 0.f),
		glm::vec3(1.f, 1.f, 0.f),
	});

	attribute.texCoords[0].assign({
		rect.GetLeftBottom(),
		rect.GetRightBottom(),
		rect.GetLeftTop(),
		rect.GetRightTop(),
	});

	attribute.indexes.assign({ 0, 1, 2, 3 });

	Mesh mesh = new IMesh();
	mesh->SetAttribute(attribute);
	mesh->AddSubMesh(new ISubMesh());

	TriangleBias bias{ attribute.indexes.size() };
	mesh->GetSubMesh(0)->SetTriangleBias(bias);

	return mesh;
}

void GraphicsInternal::DrawSubMeshesIfPassEnabled(Mesh mesh, Material material, uint pass) {
	if (material->IsPassEnabled(pass)) {
		material->Bind(pass);

		for (SubMesh subMesh : mesh->GetSubMeshes()) {
			GLUtils::DrawElementsBaseVertex(mesh->GetTopology(), subMesh->GetTriangleBias());
		}

		material->Unbind();
	}
}

void GraphicsInternal::DrawSubMeshIfPassEnabled(Mesh mesh, uint subMeshIndex, Material material, uint pass) {
	if (material->IsPassEnabled(pass)) {
		material->Bind(pass);
		GLUtils::DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(subMeshIndex)->GetTriangleBias());
		material->Unbind();
	}
}

Material GraphicsInternal::CreateBlitMaterial() {
	Shader shader = new IShader();
	shader->Load("builtin/blit");

	Material material = new IMaterial();
	material->SetShader(shader);

	return material;
}
