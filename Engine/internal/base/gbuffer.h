#pragma once
#include "texture.h"

class Framebuffer;

class GBuffer {
public:
	enum GTexture {
		Pos,
		Color,
		Normal,
		GTextureCount,
	};

	enum GPass {
		GeometryPass,
		StencilPass,
		LightPass,
		FinalPass,
	};

public:
	GBuffer();
	~GBuffer();

public:
	bool Create(uint width, uint height);

	void Bind(GPass pass);
	void Unbind();

	void Clear();
	RenderTexture GetRenderTexture(GTexture index);
	RenderTexture GetDepthTexture() { return depthTexture_; }

private:
	bool InitializeFramebuffer(uint width, uint height);

private:
	Framebuffer* framebuffer_;
	RenderTexture depthTexture_;
	RenderTexture finalTexture_;
	RenderTexture textures_[GTextureCount];

	int colorAttachments_[GTextureCount];
};