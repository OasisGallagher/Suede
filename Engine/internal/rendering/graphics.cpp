#include "graphics.h"
#include "builtinproperties.h"

#include "internal/base/context.h"
#include "internal/rendering/pipeline.h"

struct Details {
	Details() {
		blitMaterial = new Material();
		blitMaterial->SetShader(Shader::Find("builtin/blit"));
	}

	ShadingMode shadingMode = ShadingMode::Shaded;
	ref_ptr<Material> blitMaterial;
	bool ambientOcclusionEnabled = false;
};

static Details& d() {
	static Details details;
	return details;
}

static ref_ptr<Mesh> CreateBlitMesh(const Rect& rect) {
	MeshAttribute attribute = { MeshTopology::TriangleStripe };

	attribute.positions.assign({
		Vector3(-1.f, -1.f, 0.f),
		Vector3(1.f, -1.f, 0.f),
		Vector3(-1.f, 1.f, 0.f),
		Vector3(1.f, 1.f, 0.f),
	});

	attribute.texCoords[0].assign({
		rect.GetLeftBottom(),
		rect.GetRightBottom(),
		rect.GetLeftTop(),
		rect.GetRightTop(),
	});

	attribute.indexes.assign({ 0, 1, 2, 3 });

	return Mesh::FromAttribute(attribute);
}

static void DrawSubMeshes(Mesh* mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		Context::GetCurrent()->DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(i)->GetTriangleBias());
	}
}

void Graphics::SetShadingMode(ShadingMode value) {
	if (d().shadingMode != value) {
		Context::GetCurrent()->PolygonMode(GL_FRONT_AND_BACK, value == ShadingMode::Shaded ? GL_FILL : GL_LINE);
		d().shadingMode = value;
	}
}

ShadingMode Graphics::GetShadingMode() {
	return d().shadingMode;
}

void Graphics::SetAmbientOcclusionEnabled(bool value) { d().ambientOcclusionEnabled = value; }
bool Graphics::GetAmbientOcclusionEnabled() { return d().ambientOcclusionEnabled; }

void Graphics::Blit(Texture* src, RenderTexture* dest) {
	Blit(src, dest, d().blitMaterial.get(), Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void Graphics::Blit(Texture* src, RenderTexture* dest, const Rect& rect) {
	Blit(src, dest, d().blitMaterial.get(), rect, rect);
}

void Graphics::Blit(Texture* src, RenderTexture* dest, const Rect& srcRect, const Rect& destRect) {
	Blit(src, dest, d().blitMaterial.get(), srcRect, destRect);
}

void Graphics::Blit(Texture* src, RenderTexture* dest, Material* material) {
	Blit(src, dest, material, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void Graphics::Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& rect) {
	Blit(src, dest, material, rect, rect);
}

void Graphics::Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& srcRect, const Rect& destRect) {
	if (!dest) { dest = RenderTexture::GetDefault(); }

	dest->BindWrite(destRect);
	material->SetTexture(BuiltinProperties::MainTexture, src);

	ref_ptr<Mesh> mesh = CreateBlitMesh(srcRect);
	Draw(mesh.get(), material);

	dest->Unbind();
}

void Graphics::SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer) {
}

void Graphics::Draw(Mesh* mesh, Material* material) {
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
