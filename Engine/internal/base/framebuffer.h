#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "api/gl.h"
#include "texture.h"

enum FramebufferTarget {
	FramebufferTargetRead,
	FramebufferTargetWrite,
	FramebufferTargetReadWrite,
};

enum FramebufferClearMask {
	FramebufferClearMaskNone = 0,
	FramebufferClearMaskColor = 1,
	FramebufferClearMaskDepth = 2,
	FramebufferClearMaskStencil = 4,

	FramebufferClearMaskColorDepth = FramebufferClearMaskColor | FramebufferClearMaskDepth,
	FramebufferClearMaskColorDepthStencil = FramebufferClearMaskColorDepth | FramebufferClearMaskStencil,
};

enum FramebufferAttachment {
	FramebufferAttachmentNone = -1,
	FramebufferAttachment0, 
	FramebufferAttachment1,
	FramebufferAttachment2,
	FramebufferAttachment3,
	FramebufferAttachment4,
	FramebufferAttachment5,
	FramebufferAttachment6,
	FramebufferAttachment7,
	FramebufferAttachment8,
	FramebufferAttachmentMax = FramebufferAttachment8,
};

class FramebufferBase;
struct FramebufferState {
	void BindWrite(FramebufferClearMask clearMask);
	void Unbind();
	void Clear();

	bool operator == (const FramebufferState& other) const;
	bool operator != (const FramebufferState& other) const;

	FramebufferBase* framebuffer;
	RenderTexture depthTexture;

	RenderTexture renderTexture;
	FramebufferAttachment attachment;

private:
	RenderTexture oldDepthTexture;
	RenderTexture oldRenderTexture;
};

class FramebufferBase {
public:
	virtual void ReadBuffer(std::vector<uchar>& data);

	virtual void BindWrite(FramebufferClearMask clearMask);
	virtual void Unbind();

	virtual void Clear(FramebufferClearMask clearMask);

public:
	virtual void SetDepthTexture(RenderTexture texture);
	virtual void CreateDepthRenderbuffer();

	virtual uint GetRenderTextureCount();
	virtual RenderTexture GetDepthTexture();

	virtual RenderTexture GetRenderTexture(FramebufferAttachment attachment);
	virtual void SetRenderTexture(FramebufferAttachment attachment, RenderTexture texture);

public:
	void SaveState(FramebufferState& state);

public:
	int GetViewportWidth() const { return width_; }
	int GetViewportHeight() const { return height_; }

	void SetViewport(uint width, uint height);

	void SetClearColor(const glm::vec3& value) { clearColor_ = value; }
	glm::vec3 GetClearColor() const { return clearColor_; }

	void SetClearDepth(float value) { clearDepth_ = value; }
	float GetClearDepth() const { return clearDepth_; }

	uint GetNativePointer() { return fbo_; }

protected:
	FramebufferBase();
	virtual ~FramebufferBase() {}

protected:
	virtual void OnViewportChanged() {}

protected:
	void ClearCurrent(FramebufferClearMask clearMask);

	void BindFramebuffer(FramebufferTarget target = FramebufferTargetReadWrite);
	void UnbindFramebuffer();

	void BindViewport();
	void UnbindViewport();

	GLbitfield FramebufferClearBitmaskToGLbitfield(FramebufferClearMask clearMask);
	void FramebufferTargetToGLenum(FramebufferTarget target, GLenum* query, GLenum* bind);

protected:
	GLuint fbo_;
	GLsizei width_;
	GLsizei height_;

	float clearDepth_;
	glm::vec3 clearColor_;

private:
	GLint oldFramebuffer_;
	GLenum bindTarget_;
};

class Framebuffer0 : public FramebufferBase {
public:
	static Framebuffer0* Get();

private:
	Framebuffer0() {}
};

class Framebuffer : public FramebufferBase {
public:
	Framebuffer();
	~Framebuffer();

public:
	void Create(int width, int height);

	void BindRead(FramebufferAttachment attachment);

	virtual void BindWrite(FramebufferClearMask clearMask);
	virtual void Clear(FramebufferClearMask clearMask);

	void ClearAttachment(FramebufferClearMask clearMask, FramebufferAttachment attachment) { ClearAttachments(clearMask, 1, &attachment); }
	void ClearAttachments(FramebufferClearMask clearMask, uint n, FramebufferAttachment* attachments);

	void BindWriteAttachment(FramebufferAttachment attachment) { BindWriteAttachments(1, &attachment); }
	void BindWriteAttachments(uint n, FramebufferAttachment* attachments);

public:
	virtual void SetDepthTexture(RenderTexture texture);
	virtual void CreateDepthRenderbuffer();

	virtual uint GetRenderTextureCount();
	virtual RenderTexture GetDepthTexture();

	virtual RenderTexture GetRenderTexture(FramebufferAttachment attachment);
	virtual void SetRenderTexture(FramebufferAttachment attachment, RenderTexture texture);

private:
	virtual void OnViewportChanged();
	
private:
	uint ToGLColorAttachments();
	uint ToGLColorAttachments(uint n, FramebufferAttachment* attachments);
	GLenum FramebufferAttachmentToGLenum(FramebufferAttachment attachment);

	void ClearCurrentAllAttachments(FramebufferClearMask clearMask);
	void ClearBuffers(FramebufferClearMask clearMask, uint n, GLenum* buffers);
	void ClearCurrentAttachments(FramebufferClearMask clearMask, uint n, FramebufferAttachment* attachments);

private:
	GLuint depthRenderbuffer_;

	int attachedRenderTextureCount_;
	int maxRenderTextures_;
	GLenum* glAttachments_;
	RenderTexture* renderTextures_;
	RenderTexture depthTexture_;
};
