#pragma once
#include "texture.h"
#include "framebuffer.h"

class GBuffer {
public:
	enum GTexture {
		Pos,
		Albedo,
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
	Framebuffer framebuffer_;
	RenderTexture depthTexture_;
	RenderTexture finalTexture_;
	RenderTexture textures_[GTextureCount];

	FramebufferAttachment colorAttachments_[GTextureCount];
};