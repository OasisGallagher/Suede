#pragma once
#include "graphics.h"
#include "internal/engine/subsysteminternal.h"

class GraphicsInternal : public SubsystemInternal {
public:
	void Awake();

	void SetShadingMode(ShadingMode value);
	ShadingMode GetShadingMode();

	void SetAmbientOcclusionEnabled(bool value);
	bool GetAmbientOcclusionEnabled();

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
	ref_ptr<Mesh> CreateBlitMesh(const Rect& rect);

private:
	ref_ptr<Material> blitMaterial_;
	bool ambientOcclusionEnabled_ = false;
	ShadingMode shadingMode_ = ShadingMode::Shaded;
};
