#pragma once
#include "graphics.h"

class GraphicsInternal : public Graphics {
public:
	GraphicsInternal();

public:
	virtual void SetShadingMode(ShadingMode value);
	virtual ShadingMode GetShadingMode() { return mode_; }

	virtual void EnableAmbientOcclusion(bool value) { ambientOcclusionEnabled_ = value; }
	virtual bool IsAmbientOcclusionEnabled() { return ambientOcclusionEnabled_; }

	virtual void SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer);

	virtual void Draw(Mesh mesh, Material material);

	virtual void Blit(Texture src, RenderTexture dest);
	virtual void Blit(Texture src, RenderTexture dest, const Rect& rect);
	virtual void Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect);

	virtual void Blit(Texture src, RenderTexture dest, Material material);
	virtual void Blit(Texture src, RenderTexture dest, Material material, const Rect& rect);
	virtual void Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect);

private:
	void DrawSubMeshes(Mesh mesh);
	Material CreateBlitMaterial();
	Mesh CreateBlitMesh(const Rect& rect);

private:
	Material material_;
	ShadingMode mode_;
	bool ambientOcclusionEnabled_;
};
