#include "rect.h"
#include "texture.h"
#include "material.h"

class Graphics;
class AmbientOcclusion {
public:
	AmbientOcclusion(Graphics* graphics, RenderTexture* target);

public:
	void Run(RenderTexture* depthTexture, const Rect& normalizedRect);
	void Clear(const Rect& normalizedRect);
	void Resize(int width, int height) { ssaoRT_->Resize(width, height); }

	Material* GetMaterial() { return ssaoMaterial_.get(); }
	Material* GetTraversalMaterial() { return ssaoTraversalMaterial_.get(); }

	RenderTexture* GetRenderTexture() { return ssaoRT_; }
	RenderTexture* GetTraversalRenderTexture() { return ssaoTraversalRT_.get(); }

private:
	void InitializeKernel();

private:
	Graphics* graphics_;
	ref_ptr<Material> ssaoMaterial_;
	ref_ptr<Material> ssaoTraversalMaterial_;

	RenderTexture* ssaoRT_;
	ref_ptr<MRTRenderTexture> ssaoTraversalRT_;
};
