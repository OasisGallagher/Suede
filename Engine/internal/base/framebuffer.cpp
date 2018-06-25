#include "screen.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "framebuffer.h"
#include "api/glutils.h"
#include "memory/memory.h"

#define LogUnsupportedFramebufferOperation()	Debug::LogError("unsupported framebuffer operation %s.", __func__);

FramebufferBase::FramebufferBase() : fbo_(0), oldFramebuffer_(0)
	, bindTarget_(0), clearDepth_(1), clearStencil_(0) {
	viewport_ = glm::uvec4(0, 0, Screen::GetWidth(), Screen::GetHeight());
}

void FramebufferBase::BindRead() {
	BindFramebuffer(FramebufferTargetRead);
	BindViewport();
}

void FramebufferBase::BindWrite() {
	BindFramebuffer(FramebufferTargetWrite);
	BindViewport();
}

void FramebufferBase::Unbind() {
	UnbindFramebuffer();
	UnbindViewport();
}

void FramebufferBase::ReadBuffer(std::vector<uchar>& data, uint* alignment) {
	BindRead();
	ReadCurrentBuffer(data, alignment);
	Unbind();
}

void FramebufferBase::ReadCurrentBuffer(std::vector<uchar> &data, uint* alignment) {
	uint packAlignment = GLUtils::GetGLMode(GLModePackAlignment);

	uint width = Math::RoundUpToPowerOfTwo(viewport_.z, packAlignment);
	data.resize(3 * width * viewport_.w);

	GL::ReadPixels(0, 0, viewport_.z, viewport_.w, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);

	if (alignment != nullptr) {
		*alignment = packAlignment;
	}
}

void FramebufferBase::BindFramebuffer(FramebufferTarget target) {
	if (bindTarget_ != 0) {
		Debug::LogError("framebuffer already bound.");
		return;
	}

	GLenum query, bind;
	FramebufferTargetToGLenum(target, &query, &bind);

	GL::GetIntegerv(query, &oldFramebuffer_);
	GL::BindFramebuffer(bind, fbo_);

	bindTarget_ = bind;
}

void FramebufferBase::UnbindFramebuffer() {
	GL::BindFramebuffer(bindTarget_, oldFramebuffer_);
	bindTarget_ = oldFramebuffer_ = 0;
}

void FramebufferBase::BindViewport() {
	GL::Viewport(viewport_.x, viewport_.y, viewport_.z, viewport_.w);
}

void FramebufferBase::UnbindViewport() {
}

void FramebufferBase::FramebufferTargetToGLenum(FramebufferTarget target, GLenum* query, GLenum* bind) {
	GLenum glQuery = GL_FRAMEBUFFER_BINDING, glBind = GL_FRAMEBUFFER;
	switch (target) {
		case FramebufferTargetRead:
			glQuery = GL_READ_FRAMEBUFFER_BINDING;
			glBind = GL_READ_FRAMEBUFFER;
			break;
		case FramebufferTargetWrite:
			glQuery = GL_DRAW_FRAMEBUFFER_BINDING;
			glBind = GL_DRAW_FRAMEBUFFER;
			break;
		case FramebufferTargetReadWrite:
			break;
		default:
			Debug::LogError("undefined FramebufferTarget: %d.", target);
			break;
	}

	if (query != nullptr) { *query = glQuery; }
	if (bind != nullptr) { *bind = glBind; }
}

void FramebufferBase::SetViewport(uint x, uint y, uint width, uint height) {
	glm::uvec4 value(x, y, width, height);
	if (viewport_ != value) {
		viewport_ = value;
		OnViewportChanged();
	}
}

void FramebufferBase::Clear(FramebufferClearMask clearMask) {
	BindWrite();

	GL::Enable(GL_SCISSOR_TEST);
	GL::Scissor(viewport_.x, viewport_.y, viewport_.z, viewport_.w);
	ClearCurrent(clearMask);
	GL::Disable(GL_SCISSOR_TEST);

	Unbind();
}

void FramebufferBase::SetDepthTexture(uint texture) {
	LogUnsupportedFramebufferOperation();
}

void FramebufferBase::CreateDepthRenderbuffer() {
	LogUnsupportedFramebufferOperation();
}

uint FramebufferBase::GetRenderTextureCount() {
	LogUnsupportedFramebufferOperation();
	return 0;
}

uint FramebufferBase::GetDepthTexture() {
	LogUnsupportedFramebufferOperation();
	return 0;
}

uint FramebufferBase::GetRenderTexture(FramebufferAttachment attachment) {
	LogUnsupportedFramebufferOperation();
	return 0;
}

void FramebufferBase::SetRenderTexture(FramebufferAttachment attachment, uint texture) {
	LogUnsupportedFramebufferOperation();
}

void FramebufferBase::ClearCurrent(FramebufferClearMask clearMask) {
	GLbitfield bitfield = 0;
	if ((clearMask & FramebufferClearMaskColor) != 0) {
		GL::ClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
		bitfield |= GL_COLOR_BUFFER_BIT;
	}

	if ((clearMask & FramebufferClearMaskDepth) != 0) {
		GL::ClearDepth(clearDepth_);
		bitfield |= GL_DEPTH_BUFFER_BIT;
	}

	if ((clearMask & FramebufferClearMaskStencil) != 0) {
		GL::ClearStencil(clearStencil_);
		bitfield |= GL_STENCIL_BUFFER_BIT;
	}

	if (bitfield != 0) { GL::Clear(bitfield); }
}

Framebuffer0* Framebuffer0::Get() {
	static Framebuffer0 fb0;
	return &fb0;
}

Framebuffer::Framebuffer() : depthRenderbuffer_(0), depthTexture_(0), attachedRenderTextureCount_(0) {
	viewport_ = glm::uvec4(0, 0, Screen::GetWidth(), Screen::GetHeight());

	GL::GenFramebuffers(1, &fbo_);

	renderTextures_ = MEMORY_CREATE_ARRAY(uint, GLUtils::GetLimits(GLLimitsMaxColorAttachments));
	std::fill(renderTextures_, renderTextures_ + GLUtils::GetLimits(GLLimitsMaxColorAttachments), 0);

	glAttachments_ = MEMORY_CREATE_ARRAY(GLenum, GLUtils::GetLimits(GLLimitsMaxColorAttachments));
}

Framebuffer::~Framebuffer() {
	MEMORY_RELEASE_ARRAY(renderTextures_);
	MEMORY_RELEASE_ARRAY(glAttachments_);
	
	if (fbo_ != 0) {
		GL::DeleteFramebuffers(1, &fbo_);
	}

	if (depthRenderbuffer_ != 0) {
		GL::DeleteRenderbuffers(1, &depthRenderbuffer_);
		depthRenderbuffer_ = 0;
	}
}

void Framebuffer::BindWrite() {
	FramebufferBase::BindWrite();

	uint count = ToGLColorAttachments();
	GL::DrawBuffers(count, glAttachments_);
}

void Framebuffer::ReadAttachmentBuffer(std::vector<uchar>& data, FramebufferAttachment attachment, uint* alignment) {
	BindReadAttachment(attachment);
	ReadCurrentBuffer(data, alignment);
	Unbind();
}

void Framebuffer::BindReadAttachment(FramebufferAttachment attachment) {
	FramebufferBase::BindFramebuffer(FramebufferTargetRead);
	GL::ReadBuffer(FramebufferAttachmentToGLenum(attachment));
	BindViewport();
}

void Framebuffer::BindWriteAttachments(FramebufferAttachment* attachments, uint n) {
	FramebufferBase::BindFramebuffer(FramebufferTargetWrite);
	ClearCurrent(FramebufferClearMaskColorDepthStencil);

	uint count = ToGLColorAttachments(attachments, n);
	GL::DrawBuffers(count, glAttachments_);

	BindViewport();
}

void Framebuffer::OnViewportChanged() {
	if (depthRenderbuffer_ != 0) {
		ResizeDepthRenderbuffer();
	}
}

void Framebuffer::ClearAttachments(FramebufferClearMask clearMask, FramebufferAttachment* attachments, uint n) {
	BindFramebuffer(FramebufferTargetWrite);
	ClearCurrentAttachments(clearMask, attachments, n);
	UnbindFramebuffer();
}

void Framebuffer::ClearCurrent(FramebufferClearMask clearMask) {
	uint count = ToGLColorAttachments();
	GL::DrawBuffers(count, glAttachments_);
	FramebufferBase::ClearCurrent(clearMask);
}

void Framebuffer::ClearCurrentAttachments(FramebufferClearMask clearMask, FramebufferAttachment* attachments, uint n) {
	n = ToGLColorAttachments(attachments, n);
	GL::DrawBuffers(n, glAttachments_);
	FramebufferBase::ClearCurrent(clearMask);
}

GLenum Framebuffer::FramebufferAttachmentToGLenum(FramebufferAttachment attachment) {
	if (attachment == FramebufferAttachmentNone) { return GL_NONE; }
	return attachment - FramebufferAttachment0 + GL_COLOR_ATTACHMENT0;
}

uint Framebuffer::ToGLColorAttachments() {
	if (attachedRenderTextureCount_ == 0) {
		glAttachments_[0] = GL_NONE;
		return 1;
	}

	uint count = 0;
	for (int i = 0; i < GLUtils::GetLimits(GLLimitsMaxColorAttachments); ++i) {
		if (renderTextures_[i] != 0) {
			glAttachments_[count++] = GL_COLOR_ATTACHMENT0 + i;
		}
	}

	return count;
}

uint Framebuffer::ToGLColorAttachments(FramebufferAttachment* attachments, uint n) {
	uint i = 0;
	for (; i < n; ++i) {
		glAttachments_[i] = FramebufferAttachmentToGLenum(attachments[i]);
	}

	return i;
}

void Framebuffer::CreateDepthRenderbuffer() {
	if (depthRenderbuffer_ != 0 || depthTexture_ != 0) {
		Debug::LogError("depth texture or render buffer already exists");
		return;
	}

	BindFramebuffer(FramebufferTargetWrite);
	GL::GenRenderbuffers(1, &depthRenderbuffer_);
	ResizeDepthRenderbuffer();
	GL::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);
	UnbindFramebuffer();
}

void Framebuffer::ResizeDepthRenderbuffer() {
	GLint size[2] = { 0 };
	GL::BindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);

	GL::GetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, size);
	GL::GetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, size + 1);

	if (size[0] != viewport_.x + viewport_.z || size[1] != viewport_.y + viewport_.w) {
		GL::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewport_.x + viewport_.z, viewport_.y + viewport_.w);
	}

	GL::BindRenderbuffer(GL_RENDERBUFFER, 0);
}

uint Framebuffer::GetRenderTexture(FramebufferAttachment attachment) {
	if (attachment >= GLUtils::GetLimits(GLLimitsMaxColorAttachments)) {
		Debug::LogError("index out of range");
		return 0;
	}

	return renderTextures_[attachment];
}

void Framebuffer::SetRenderTexture(FramebufferAttachment attachment, uint texture) {
	BindFramebuffer(FramebufferTargetWrite);

	if (renderTextures_[attachment] == 0 && texture != 0) {
		++attachedRenderTextureCount_;
	}
	else if (renderTextures_[attachment] != 0 && texture == 0) {
		--attachedRenderTextureCount_;
	}

	renderTextures_[attachment] = texture;

	GL::FramebufferTexture(GL_FRAMEBUFFER, FramebufferAttachmentToGLenum(attachment), texture, 0);

	UnbindFramebuffer();
}

void Framebuffer::SetDepthTexture(uint texture) {
	BindFramebuffer(FramebufferTargetWrite);
	depthTexture_ = texture;
	GL::FramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

	if (texture == 0 && depthRenderbuffer_ != 0) {
		GL::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);
	}

	GLenum status = GL::CheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE && status != GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
		Debug::LogError("failed to bind depth texture(0x%x)", status);
	}

	UnbindFramebuffer();
}

uint Framebuffer::GetRenderTextureCount() {
	return attachedRenderTextureCount_;
}

uint Framebuffer::GetDepthTexture() {
	return depthTexture_;
}
