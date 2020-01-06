#include "graphicsinternal.h"
#include "builtinproperties.h"

#include "internal/base/context.h"
#include "internal/rendering/pipeline.h"

Graphics::Graphics() : Subsystem(new GraphicsInternal()) {}
void Graphics::Awake() { _suede_dptr()->Awake(); }
void Graphics::SetShadingMode(ShadingMode value) { _suede_dptr()->SetShadingMode(value); }
ShadingMode Graphics::GetShadingMode() { return _suede_dptr()->GetShadingMode(); }
void Graphics::SetAmbientOcclusionEnabled(bool value) { _suede_dptr()->SetAmbientOcclusionEnabled(value); }
bool Graphics::GetAmbientOcclusionEnabled() { return _suede_dptr()->GetAmbientOcclusionEnabled(); }
void Graphics::SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer) { _suede_dptr()->SetRenderTarget(colorBuffers, depthBuffer); }
void Graphics::Draw(Mesh* mesh, Material* material) { _suede_dptr()->Draw(mesh, material); }
void Graphics::Blit(Texture* src, RenderTexture* dest) { _suede_dptr()->Blit(src, dest); }
void Graphics::Blit(Texture* src, RenderTexture* dest, const Rect& rect) { _suede_dptr()->Blit(src, dest, rect); }
void Graphics::Blit(Texture* src, RenderTexture* dest, const Rect& srcRect, const Rect& destRect) { _suede_dptr()->Blit(src, dest, srcRect, destRect); }
void Graphics::Blit(Texture* src, RenderTexture* dest, Material* material) { _suede_dptr()->Blit(src, dest, material); }
void Graphics::Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& rect) { _suede_dptr()->Blit(src, dest, material, rect); }
void Graphics::Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& srcRect, const Rect& destRect) { _suede_dptr()->Blit(src, dest, material, srcRect, destRect); }

// Invoke Graphics methods in current context.
#define currentContext	Context::GetCurrent()

void GraphicsInternal::Awake() {
	blitMaterial_ = new Material();
	blitMaterial_->SetShader(Shader::Find("builtin/blit"));
}

void GraphicsInternal::SetShadingMode(ShadingMode value) {
	if (shadingMode_ != value) {
		currentContext->PolygonMode(GL_FRONT_AND_BACK, value == ShadingMode::Shaded ? GL_FILL : GL_LINE);
		shadingMode_ = value;
	}
}

ShadingMode GraphicsInternal::GetShadingMode() {
	return shadingMode_;
}

void GraphicsInternal::SetAmbientOcclusionEnabled(bool value) { ambientOcclusionEnabled_ = value; }
bool GraphicsInternal::GetAmbientOcclusionEnabled() { return ambientOcclusionEnabled_; }

void GraphicsInternal::Blit(Texture* src, RenderTexture* dest) {
	Blit(src, dest, blitMaterial_.get(), Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void GraphicsInternal::Blit(Texture* src, RenderTexture* dest, const Rect& rect) {
	Blit(src, dest, blitMaterial_.get(), rect, rect);
}

void GraphicsInternal::Blit(Texture* src, RenderTexture* dest, const Rect& srcRect, const Rect& destRect) {
	Blit(src, dest, blitMaterial_.get(), srcRect, destRect);
}

void GraphicsInternal::Blit(Texture* src, RenderTexture* dest, Material* material) {
	Blit(src, dest, material, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1));
}

void GraphicsInternal::Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& rect) {
	Blit(src, dest, material, rect, rect);
}

void GraphicsInternal::Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& srcRect, const Rect& destRect) {
	if (!dest) { dest = RenderTexture::GetDefault(); }

	dest->BindWrite(destRect);
	material->SetTexture(BuiltinProperties::MainTexture, src);

	ref_ptr<Mesh> mesh = CreateBlitMesh(srcRect);
	Draw(mesh.get(), material);

	dest->Unbind();
}

ref_ptr<Mesh> GraphicsInternal::CreateBlitMesh(const Rect& rect) {
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

void GraphicsInternal::DrawSubMeshes(Mesh* mesh) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		Context::GetCurrent()->DrawElementsBaseVertex(mesh->GetTopology(), mesh->GetSubMesh(i)->GetTriangleBias());
	}
}

void GraphicsInternal::SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer) {
}

void GraphicsInternal::Draw(Mesh* mesh, Material* material) {
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
