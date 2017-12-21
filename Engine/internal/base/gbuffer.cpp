#include "gbuffer.h"
#include "framebuffer.h"
#include "internal/memory/memory.h"

GBuffer::GBuffer() {
	framebuffer_ = MEMORY_CREATE(Framebuffer);
	for (int i = 0; i < GTextureCount; ++i) {
		colorAttachments_[i] = FramebufferAttachment0 + i;
	}
}

GBuffer::~GBuffer() {
	MEMORY_RELEASE(framebuffer_);
}

bool GBuffer::Create(uint width, uint height) {
	return InitializeFramebuffer(width, height);
}

void GBuffer::Clear() {
	framebuffer_->Clear(FramebufferClearBitmaskColor);// , FramebufferAttachment0 + GTextureCount);
}

void GBuffer::Bind(GPass pass) {
	switch (pass) {
		case GeometryPass:
			framebuffer_->BindWrite(GTextureCount, colorAttachments_);
			break;
		case StencilPass:
			framebuffer_->BindWrite(FramebufferAttachmentNone);
			break;
		case LightPass:
			framebuffer_->BindWrite(FramebufferAttachment0 + GTextureCount);
			break;
		case FinalPass:
			Framebuffer0::Get()->BindWrite();
			framebuffer_->BindRead(FramebufferAttachment0 + GTextureCount);
			break;
	}
}

void GBuffer::Unbind() {
	framebuffer_->Unbind();
}

RenderTexture GBuffer::GetRenderTexture(GTexture index) {
	if (index >= GTextureCount) {
		Debug::LogError("index out of range.");
		return nullptr;
	}

	return textures_[index];
}

bool GBuffer::InitializeFramebuffer(uint width, uint height) {
	framebuffer_->Create(width, height);

	for (int i = 0; i < GTextureCount; ++i) {
		textures_[i] = NewRenderTexture();
		textures_[i]->Load(RenderTextureFormatRgba, width, height);
		framebuffer_->SetRenderTexture(FramebufferAttachment0 + i, textures_[i]);
	}

	depthTexture_ = NewRenderTexture();
	depthTexture_->Load(RenderTextureFormatDepthStencil, width, height);
	framebuffer_->SetDepthTexture(depthTexture_);

	finalTexture_ = NewRenderTexture();
	finalTexture_->Load(RenderTextureFormatRgba, width, height);
	framebuffer_->SetRenderTexture(FramebufferAttachment0 + GTextureCount, finalTexture_);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Debug::LogError("failed to create gbuffer.");
		return false;
	}

	return true;
}
