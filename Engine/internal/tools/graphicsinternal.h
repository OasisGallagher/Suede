#pragma once
#include "graphics.h"

class GraphicsInternal {
public:
	GraphicsInternal();

public:
	void SetShadingMode(ShadingMode value);
	ShadingMode GetShadingMode() { return mode_; }

	void SetAmbientOcclusionEnabled(bool value) { ambientOcclusionEnabled_ = value; }
	bool GetAmbientOcclusionEnabled() { return ambientOcclusionEnabled_; }

	void SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer);

	void Draw(Mesh* mesh, Material* material);

	void Blit(Texture* src, RenderTexture* dest);
	void Blit(Texture* src, RenderTexture* dest, const Rect& rect);
	void Blit(Texture* src, RenderTexture* dest, const Rect& srcRect, const Rect& destRect);

	void Blit(Texture* src, RenderTexture* dest, Material* material);
	void Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& rect);
	void Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& srcRect, const Rect& destRect);

private:
	void DrawSubMeshes(Mesh* mesh);
	ref_ptr<Material> CreateBlitMaterial();
	ref_ptr<Mesh> CreateBlitMesh(const Rect& rect);

private:
	ShadingMode mode_;
	ref_ptr<Material> material_;
	bool ambientOcclusionEnabled_;
};
