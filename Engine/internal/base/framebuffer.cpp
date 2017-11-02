#include "tools/debug.h"
#include "framebuffer.h"
#include "tools/string.h"
#include "internal/memory/memory.h"
#include "internal/memory/factory.h"
#include "internal/base/textureinternal.h"

Framebuffer0::Framebuffer0() : oldFramebuffer_(0) {
}

void Framebuffer0::Create(int width, int height) {
	width_ = width, height_ = height;
	fbo_ = 0;
}

void Framebuffer0::Bind() {
	Clear();
	BindFramebuffer();
	BindViewport();
}

void Framebuffer0::Unbind() {
	UnbindFramebuffer();
	UnbindViewport();
}

void Framebuffer0::ReadBuffer(std::vector<unsigned char>& pixels) {
	BindFramebuffer();
	
	int oldPackAlignment = 4;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldPackAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	pixels.resize(4 * GetWidth() * GetHeight());
	glReadPixels(0, 0, GetWidth(), GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, oldPackAlignment);
	UnbindFramebuffer();
}

void Framebuffer0::BindFramebuffer() {
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void Framebuffer0::UnbindFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer_);
	oldFramebuffer_ = 0;
}

void Framebuffer0::BindViewport() {
	glViewport(0, 0, GetWidth(), GetHeight());
}

void Framebuffer0::UnbindViewport() {
}

void Framebuffer0::Resize(int w, int h) {
	if (width_ == w && height_ == h) { return; }
	width_ = w;
	height_ = h;
}

void Framebuffer0::Clear(int buffers) {
	BindFramebuffer();
	glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
	glClear(buffers);
	UnbindFramebuffer();
}

Framebuffer::Framebuffer() : depthRenderbuffer_(0){
}

Framebuffer::~Framebuffer() {
	Memory::ReleaseArray(renderTextures_);
	Memory::ReleaseArray(attachments_);

	if (depthRenderbuffer_ != 0) {
		glDeleteRenderbuffers(1, &depthRenderbuffer_);
		depthRenderbuffer_ = 0;
	}
}

void Framebuffer::Create(int width, int height) {
	Framebuffer0::Create(width, height);

	attachedRenderTextureCount_ = 0;

	glGenFramebuffers(1, &fbo_);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxRenderTextures_);
	renderTextures_ = Memory::CreateArray<RenderTexture>(maxRenderTextures_);
	attachments_ = Memory::CreateArray<GLenum>(maxRenderTextures_);
}

void Framebuffer::Bind() {
	Framebuffer0::Bind();

	int count = UpdateAttachments();
	glDrawBuffers(count, attachments_);
}

int Framebuffer::UpdateAttachments() {
	if (attachedRenderTextureCount_ == 0) {
		attachments_[0] = GL_NONE;
		return 1;
	}

	int count = 0;
	for (int i = 0; i < maxRenderTextures_; ++i) {
		if (renderTextures_[i]) {
			attachments_[count++] = GL_COLOR_ATTACHMENT0 + i;
		}
	}

	return count;
}

void Framebuffer::CreateDepthRenderBuffer() {
	AssertX(depthRenderbuffer_ == 0, "depth texture or render buffer already exists");
	BindFramebuffer();

	glGenRenderbuffers(1, &depthRenderbuffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetWidth(), GetHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);

	UnbindFramebuffer();
}

void Framebuffer::SetDepthTexture(RenderTexture texture) {
	BindFramebuffer();
	depthTexture_ = texture;
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture ? texture->GetNativePointer() : 0, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Assert(status == GL_FRAMEBUFFER_COMPLETE);
	UnbindFramebuffer();
}

int Framebuffer::GetRenderTextureCount() {
	return attachedRenderTextureCount_;
}

RenderTexture Framebuffer::GetDepthTexture() {
	return depthTexture_;
}

#ifdef MRT
RenderTexture Framebuffer::GetRenderTexture(int index) {
	AssertX(index < attachedRenderTextureCount_, "index out of range");
	return renderTextures_[index];
}

void Framebuffer::AddRenderTexture(RenderTexture texture) {
	int index = FindAttachmentIndex();
	AssertX(index >= 0, "too many render textures");

	BindFramebuffer();

	renderTextures_[index] = texture;

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture->GetNativePointer(), 0);

	++attachedRenderTextureCount_;

	UnbindFramebuffer();
}

void Framebuffer::RemoveRenderTexture(RenderTexture texture) {
	if (!texture) { return; }

	for (int i = 0; i < maxRenderTextures_; ++i) {
		if (renderTextures_[i] == texture) {
			renderTextures_[i].reset();
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 0, 0);
		}
	}
}

#else

void Framebuffer::SetRenderTexture(RenderTexture texture) {
	BindFramebuffer();

	renderTextures_[0] = texture;

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture ? texture->GetNativePointer() : 0, 0);

	attachedRenderTextureCount_ = texture ? 1 : 0;
	
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	AssertX(status == GL_FRAMEBUFFER_COMPLETE, String::Format("failed to create framebuffer, 0x04x.", status));

	UnbindFramebuffer();
}

RenderTexture Framebuffer::GetRenderTexture() {
	return renderTextures_[0];
}

#endif	// MRT

int Framebuffer::FindAttachmentIndex() {
	for (int i = 0; i < maxRenderTextures_; ++i) {
		if (!renderTextures_[i]) {
			return i;
		}
	}

	return -1;
}
