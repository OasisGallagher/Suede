#include "gbuffer.h"
#include "framebuffer.h"
#include "internal/memory/memory.h"

GBuffer::GBuffer() {
	for (int i = 0; i < GTextureCount; ++i) {
		colorAttachments_[i] = FramebufferAttachment(FramebufferAttachment0 + i);
	}
}

GBuffer::~GBuffer() {
}

bool GBuffer::Create(uint width, uint height) {
	return InitializeFramebuffer(width, height);
}

void GBuffer::Clear() {
	framebuffer_.Clear(FramebufferClearBitmaskColor);// , FramebufferAttachment0 + GTextureCount);
}

void GBuffer::Bind(GPass pass) {
	switch (pass) {
		case GeometryPass:
			framebuffer_.BindWriteAttachments(GTextureCount, colorAttachments_);
			break;
		case StencilPass:
			framebuffer_.BindWriteAttachment(FramebufferAttachmentNone);
			break;
		case LightPass:
			framebuffer_.BindWriteAttachment(FramebufferAttachment(FramebufferAttachment0 + GTextureCount));
			break;
		case FinalPass:
			Framebuffer0::Get()->BindWrite();
			framebuffer_.BindRead(FramebufferAttachment(FramebufferAttachment0 + GTextureCount));
			break;
	}
}

void GBuffer::Unbind() {
	framebuffer_.Unbind();
}

RenderTexture GBuffer::GetRenderTexture(GTexture index) {
	if (index >= GTextureCount) {
		Debug::LogError("index out of range.");
		return nullptr;
	}

	return textures_[index];
}

bool GBuffer::InitializeFramebuffer(uint width, uint height) {
	framebuffer_.Create(width, height);

	for (int i = 0; i < GTextureCount; ++i) {
		textures_[i] = NewRenderTexture();
		textures_[i]->Load(RenderTextureFormatRgbHdr, width, height);
		framebuffer_.SetRenderTexture(FramebufferAttachment(FramebufferAttachment0 + i), textures_[i]);
	}

	depthTexture_ = NewRenderTexture();
	depthTexture_->Load(RenderTextureFormatDepthStencil, width, height);
	framebuffer_.SetDepthTexture(depthTexture_);

	finalTexture_ = NewRenderTexture();
	finalTexture_->Load(RenderTextureFormatRgba, width, height);
	framebuffer_.SetRenderTexture(FramebufferAttachment(FramebufferAttachment0 + GTextureCount), finalTexture_);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Debug::LogError("failed to create gbuffer.");
		return false;
	}

	return true;
}
