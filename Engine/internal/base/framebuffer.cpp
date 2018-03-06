#include "debug/debug.h"
#include "tools/math2.h"
#include "framebuffer.h"
#include "memory/memory.h"

FramebufferBase* Framebuffer::read_ = Framebuffer0::Get();
FramebufferBase* Framebuffer::write_ = Framebuffer0::Get();

bool FramebufferState::operator!=(const FramebufferState& other) const {
	return framebuffer != other.framebuffer || depthTexture != other.depthTexture
		|| renderTexture != other.renderTexture;
}

void FramebufferState::BindWrite() {
	framebuffer->SetDepthTexture(depthTexture);
	framebuffer->SetRenderTexture(attachment, renderTexture);
	framebuffer->BindWrite();
}

void FramebufferState::Unbind() {
	framebuffer->Unbind();
}

void FramebufferState::Clear() {
	framebuffer = Framebuffer0::Get();
	depthTexture = nullptr;
	renderTexture = nullptr;
	attachment = FramebufferAttachment0;
}

FramebufferBase::FramebufferBase() : oldFramebuffer_(0), bindTarget_(0) {
}

void FramebufferBase::BindWrite() {
	BindFramebuffer(FramebufferTargetWrite);
	ClearCurrent(FramebufferClearBitmaskColorDepthStencil);
	BindViewport();
}

void FramebufferBase::Unbind() {
	UnbindFramebuffer();
	UnbindViewport();
}

void FramebufferBase::ReadBuffer(std::vector<uchar>& data) {
	BindFramebuffer(FramebufferTargetRead);
	
	uint alignment = 4;
	GL::GetIntegerv(GL_PACK_ALIGNMENT, (GLint*)&alignment);

	uint w = Math::RoundUpToPowerOfTwo(GetViewportWidth(), alignment);
	data.resize(3 * w * GetViewportHeight());

	GL::ReadPixels(0, 0, GetViewportWidth(), GetViewportHeight(), GL_RGB, GL_UNSIGNED_BYTE, &data[0]);

	UnbindFramebuffer();
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
	GL::Viewport(0, 0, GetViewportWidth(), GetViewportHeight());
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

GLbitfield FramebufferBase::FramebufferClearBitmaskToGLbitfield(FramebufferClearBitmask bitmask) {
	GLbitfield bitfield = 0;
	if ((bitmask & FramebufferClearBitmaskColor) != 0) { bitfield |= GL_COLOR_BUFFER_BIT; }
	if ((bitmask & FramebufferClearBitmaskDepth) != 0) { bitfield |= GL_DEPTH_BUFFER_BIT; }
	if ((bitmask & FramebufferClearBitmaskStencil) != 0) {bitfield |= GL_STENCIL_BUFFER_BIT; }

	return bitfield;
}

void FramebufferBase::SetViewport(uint width, uint height) {
	if (width_ != width || height_ != width) {
		width_ = width;
		height_ = height;
		OnViewportChanged();
	}
}

void FramebufferBase::Clear(FramebufferClearBitmask bitmask) {
	BindFramebuffer();
	ClearCurrent(bitmask);
	UnbindFramebuffer();
}

void FramebufferBase::SetDepthTexture(RenderTexture texture) {
}

void FramebufferBase::CreateDepthRenderbuffer() {
}

uint FramebufferBase::GetRenderTextureCount() {
	return 0;
}

RenderTexture FramebufferBase::GetDepthTexture() {
	return nullptr;
}

RenderTexture FramebufferBase::GetRenderTexture(FramebufferAttachment attachment) {
	return nullptr;
}

void FramebufferBase::SetRenderTexture(FramebufferAttachment attachment, RenderTexture texture) {
}

void FramebufferBase::SaveState(FramebufferState& state) {
	state.framebuffer = this;
	state.depthTexture = GetDepthTexture();

	state.attachment = FramebufferAttachment0;
	state.renderTexture = (GetRenderTextureCount() > 0) ? GetRenderTexture(FramebufferAttachment0) : nullptr;
}

void FramebufferBase::ClearCurrent(FramebufferClearBitmask bitmask) {
	GL::ClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
	GL::Clear(FramebufferClearBitmaskToGLbitfield(bitmask));
}

Framebuffer0* Framebuffer0::Get() {
	static Framebuffer0 fb0;
	return &fb0;
}

Framebuffer::Framebuffer() : depthRenderbuffer_(0), renderTextures_(nullptr), glAttachments_(nullptr) {
}

Framebuffer::~Framebuffer() {
	MEMORY_RELEASE_ARRAY(renderTextures_);
	MEMORY_RELEASE_ARRAY(glAttachments_);

	if (depthRenderbuffer_ != 0) {
		GL::DeleteRenderbuffers(1, &depthRenderbuffer_);
		depthRenderbuffer_ = 0;
	}
}

void Framebuffer::SetCurrentRead(FramebufferBase* value) {
	if (value == nullptr) { value = Framebuffer0::Get(); }
	read_ = value;
}

void Framebuffer::SetCurrentWrite(FramebufferBase * value) {
	if (value == nullptr) { value = Framebuffer0::Get(); }
	write_ = value;
}

void Framebuffer::Create(int width, int height) {
	if (renderTextures_ != nullptr || glAttachments_ != nullptr) {
		Debug::LogError("framebuffer already created");
		return;
	}

	width_ = width;
	height_ = height;

	attachedRenderTextureCount_ = 0;

	GL::GenFramebuffers(1, &fbo_);
	GL::GetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxRenderTextures_);
	renderTextures_ = MEMORY_CREATE_ARRAY(RenderTexture, maxRenderTextures_);
	glAttachments_ = MEMORY_CREATE_ARRAY(GLenum, maxRenderTextures_);

	BindFramebuffer();
	GL::ClearDepth(1);
	GL::DepthRange(0.f, 1.f);
	UnbindFramebuffer();
}

void Framebuffer::BindWrite() {
	FramebufferBase::BindWrite();

	uint count = ToGLColorAttachments();
	GL::DrawBuffers(count, glAttachments_);
}

void Framebuffer::BindRead(FramebufferAttachment attachment) {
	FramebufferBase::BindFramebuffer(FramebufferTargetRead);
	GL::ReadBuffer(FramebufferAttachmentToGLenum(attachment));

	BindViewport();
}

void Framebuffer::BindWriteAttachments(uint n, FramebufferAttachment* attachments) {
	FramebufferBase::BindFramebuffer(FramebufferTargetWrite);
	ClearCurrentAllAttachments(FramebufferClearBitmaskColorDepthStencil);

	uint count = ToGLColorAttachments(n, attachments);
	GL::DrawBuffers(count, glAttachments_);

	BindViewport();
}

void Framebuffer::Clear(FramebufferClearBitmask bitmask) {
	BindFramebuffer();
	ClearCurrentAllAttachments(bitmask);

	UnbindFramebuffer();
}

void Framebuffer::OnViewportChanged() {
	if (depthRenderbuffer_ != 0) {
		BindFramebuffer();

		GL::BindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
		GL::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetViewportWidth(), GetViewportHeight());
		GL::BindRenderbuffer(GL_RENDERBUFFER, 0);

		UnbindFramebuffer();
	}
}

void Framebuffer::ClearAttachments(FramebufferClearBitmask bitmask, uint n, FramebufferAttachment* attachments) {
	BindFramebuffer();
	ClearCurrentAttachments(bitmask, n, attachments);
	UnbindFramebuffer();
}

void Framebuffer::ClearCurrentAllAttachments(FramebufferClearBitmask bitmask) {
	uint count = ToGLColorAttachments();
	ClearBuffers(bitmask, count, glAttachments_);
}

void Framebuffer::ClearCurrentAttachments(FramebufferClearBitmask bitmask, uint n, FramebufferAttachment* attachments) {
	n = ToGLColorAttachments(n, attachments);
	ClearBuffers(bitmask, n, glAttachments_);
}

void Framebuffer::ClearBuffers(FramebufferClearBitmask bitmask, uint n, GLenum* buffers) {
	GL::DrawBuffers(n, buffers);
	GL::ClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
	GL::Clear(FramebufferClearBitmaskToGLbitfield(bitmask));
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
	for (int i = 0; i < maxRenderTextures_; ++i) {
		if (renderTextures_[i]) {
			glAttachments_[count++] = GL_COLOR_ATTACHMENT0 + i;
		}
	}

	return count;
}

uint Framebuffer::ToGLColorAttachments(uint n, FramebufferAttachment* attachments) {
	uint i = 0;
	for (; i < n; ++i) {
		glAttachments_[i] = FramebufferAttachmentToGLenum(attachments[i]);
	}

	return i;
}

void Framebuffer::CreateDepthRenderbuffer() {
	if (depthRenderbuffer_ != 0) {
		Debug::LogError("depth texture or render buffer already exists");
		return;
	}

	BindFramebuffer();

	GL::GenRenderbuffers(1, &depthRenderbuffer_);
	GL::BindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);

	GL::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetViewportWidth(), GetViewportHeight());
	GL::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);

	GL::BindRenderbuffer(GL_RENDERBUFFER, 0);

	UnbindFramebuffer();
}

RenderTexture Framebuffer::GetRenderTexture(FramebufferAttachment attachment) {
	if (attachment >= maxRenderTextures_) {
		Debug::LogError("index out of range");
		return nullptr;
	}

	return renderTextures_[attachment];
}

void Framebuffer::SetRenderTexture(FramebufferAttachment attachment, RenderTexture texture) {
	BindFramebuffer();

	if (!renderTextures_[attachment] && texture) {
		++attachedRenderTextureCount_;
	}
	else if (renderTextures_[attachment] && !texture) {
		--attachedRenderTextureCount_;
	}

	renderTextures_[attachment] = texture;

	GL::FramebufferTexture(GL_FRAMEBUFFER, FramebufferAttachmentToGLenum(attachment), texture ? texture->GetNativePointer() : 0, 0);

	UnbindFramebuffer();
}

void Framebuffer::SetDepthTexture(RenderTexture texture) {
	BindFramebuffer();
	depthTexture_ = texture;
	GL::FramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture ? texture->GetNativePointer() : 0, 0);

	if (!texture && depthRenderbuffer_ != 0) {
		GL::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);
	}

	GLenum status = GL::CheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE && status != GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
		Debug::LogError("failed to bind depth texture");
	}

	UnbindFramebuffer();
}

uint Framebuffer::GetRenderTextureCount() {
	return attachedRenderTextureCount_;
}

RenderTexture Framebuffer::GetDepthTexture() {
	return depthTexture_;
}
