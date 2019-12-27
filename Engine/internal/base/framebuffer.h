#pragma once
#include <vector>

#include "texture.h"
#include "globjectmaintainer.h"

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

class Context;
class FramebufferBase : public GLObjectMaintainer {
public:
	FramebufferBase(Context* context);
	virtual ~FramebufferBase();

public:
	void BindRead();
	void ReadBuffer(std::vector<uchar>& data, uint* alignment);

	void Unbind();
	void Clear(FramebufferClearMask clearMask);

public:
	virtual void BindWrite();

public:
	virtual void SetDepthTexture(uint texture);
	virtual void CreateDepthRenderbuffer();

	virtual uint GetRenderTextureCount();
	virtual uint GetDepthTexture();
	virtual uint GetDepthRenderbuffer();

	virtual uint GetRenderTexture(FramebufferAttachment attachment);
	virtual void SetRenderTexture(FramebufferAttachment attachment, uint texture);

public:
	void SetViewport(int x, int y, uint width, uint height);
	const IVector4& GetViewport() const { return viewport_; }

	void SetClearColor(const Color& value) { clearColor_ = value; }
	Color GetClearColor() const { return clearColor_; }

	void SetClearDepth(float value) { clearDepth_ = value; }
	float GetClearDepth() const { return clearDepth_; }

	void SetClearStencil(int value) { clearStencil_ = value; }
	int GetClearStencil() const { return clearStencil_; }

	uint GetNativePointer() { return fbo_; }

protected:
	virtual void OnViewportChanged() {}
	virtual void ClearCurrent(FramebufferClearMask clearMask);

protected:
	bool IsFramebufferBound() const { return bindTarget_ != 0; }
	void BindFramebuffer(FramebufferTarget target);
	void UnbindFramebuffer();

	void BindViewport();
	void UnbindViewport();

	void ReadCurrentBuffer(std::vector<uchar> &data, uint* alignment);
	void FramebufferTargetToGLenum(FramebufferTarget target, uint* query, uint* bind);

protected:
	uint fbo_;
	IVector4 viewport_;

	float clearDepth_;
	int clearStencil_;
	Color clearColor_;

private:
	int oldFramebuffer_;
	uint bindTarget_;
};

class Framebuffer : public FramebufferBase {
public:
	Framebuffer(Context* context);
	~Framebuffer();

public:
	static FramebufferBase* GetDefault();

public:
	void BindReadAttachment(FramebufferAttachment attachment);
	void ReadAttachmentBuffer(std::vector<uchar>& data, FramebufferAttachment attachment, uint* alignment);

	void BindWriteAttachments(FramebufferAttachment* attachments, uint n);
	void ClearAttachments(FramebufferClearMask clearMask, FramebufferAttachment* attachments, uint n);

public:
	/**
	 * @brief bind all color attachments for writing.
	 */
	virtual void BindWrite();

public:
	virtual void SetDepthTexture(uint texture);
	virtual void CreateDepthRenderbuffer();

	void ResizeDepthRenderbuffer();

	virtual uint GetRenderTextureCount();
	virtual uint GetDepthTexture();
	virtual uint GetDepthRenderbuffer();

	virtual uint GetRenderTexture(FramebufferAttachment attachment);
	virtual void SetRenderTexture(FramebufferAttachment attachment, uint texture);

protected:
	virtual void OnViewportChanged();

	/**
	 * @brief clear all color attachments.
	 */
	virtual void ClearCurrent(FramebufferClearMask clearMask);

	virtual void OnContextDestroyed();

private:
	void Destroy();
	uint ToGLColorAttachments();
	uint ToGLColorAttachments(FramebufferAttachment* attachments, uint n);
	uint FramebufferAttachmentToGLenum(FramebufferAttachment attachment);

	void ClearCurrentAttachments(FramebufferClearMask clearMask, FramebufferAttachment* attachments, uint n);

private:
	uint depthTexture_;
	uint* renderTextures_;
	int attachedRenderTextureCount_;

	uint* glAttachments_;
	uint depthRenderbuffer_;
};
