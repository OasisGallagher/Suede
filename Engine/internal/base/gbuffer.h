//#pragma once
//#include "texture.h"
//#include "framebuffer.h"
//
//class GBuffer {
//public:
//	enum Texture {
//		Pos,
//		Albedo,
//		Normal,
//		GTextureCount,
//	};
//
//	enum Pass {
//		GeometryPass,
//		StencilPass,
//		LightPass,
//		FinalPass,
//	};
//
//public:
//	GBuffer();
//	~GBuffer();
//
//public:
//	bool Create(uint width, uint height);
//
//	void Bind(Pass pass);
//	void Unbind();
//
//	void Clear();
//	RenderTexture GetRenderTexture(Texture index) const;
//	RenderTexture GetDepthTexture() const { return depthTexture_; }
//
//private:
//	bool InitializeFramebuffer(uint width, uint height);
//
//private:
//	Framebuffer framebuffer_;
//	RenderTexture depthTexture_;
//	RenderTexture finalTexture_;
//	RenderTexture textures_[GTextureCount];
//
//	FramebufferAttachment colorAttachments_[GTextureCount];
//};