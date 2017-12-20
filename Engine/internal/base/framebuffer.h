#pragma once
#include <vector>

#include <glm/glm.hpp>
#include <wrappers/gl.h>

#include "texture.h"

enum FramebufferTarget {
	FramebufferTargetRead,
	FramebufferTargetWrite,
	FramebufferTargetReadWrite,
};

enum FramebufferClearBitmask {
	FramebufferClearBitmaskColor,
	FramebufferClearBitmaskDepth,

	FramebufferClearBitmaskColorDepth = -1,
};

enum {
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
};

class FramebufferBase {
public:
	virtual void ReadBuffer(std::vector<uchar>& data);

	virtual void BindWrite();
	virtual void Unbind();

	virtual void Clear(FramebufferClearBitmask bitmask);

public:
	int GetViewportWidth() { return width_; }
	int GetViewportHeight() { return height_; }

	void SetViewport(uint width, uint height);

	void SetClearColor(const glm::vec3& value) { clearColor_ = value; }
	glm::vec3 GetClearColor() { return clearColor_; }

	uint GetNativePointer() { return fbo_; }

protected:
	FramebufferBase();
	virtual ~FramebufferBase() {}

	void BindFramebuffer(FramebufferTarget target = FramebufferTargetReadWrite);
	void UnbindFramebuffer();

	void BindViewport();
	void UnbindViewport();

	GLbitfield FramebufferClearBitmaskToGLbitfield(FramebufferClearBitmask bitmask);
	void FramebufferTargetToGLenum(FramebufferTarget target, GLenum* query, GLenum* bind);

protected:
	GLuint fbo_;
	GLsizei width_;
	GLsizei height_;
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
	void BindRead(int attachment);

	virtual void BindWrite();
	virtual void Clear(FramebufferClearBitmask bitmask);
	virtual void Clear(FramebufferClearBitmask bitmask, int attachment) { Clear(bitmask, 1, &attachment); }
	virtual void Clear(FramebufferClearBitmask bitmask, uint n, int* attachments);

	void BindWrite(int attachment) { BindWrite(1, &attachment); }
	void BindWrite(uint n, int* attachments);

public:
	void SetDepthTexture(RenderTexture texture);
	void CreateDepthRenderBuffer();

	uint GetRenderTextureCount();
	RenderTexture GetDepthTexture();

	RenderTexture GetRenderTexture(int attachment);
	void SetRenderTexture(int attachment, RenderTexture texture);

private:
	uint ToGLColorAttachments();
	uint ToGLColorAttachments(uint n, int* buffers);
	void ClearBuffers(FramebufferClearBitmask bitmask, uint n, GLenum* buffers);
	GLenum FramebufferAttachmentToGLenum(int attachment);

private:
	GLuint depthRenderbuffer_;

	int attachedRenderTextureCount_;
	int maxRenderTextures_;
	GLenum* glAttachments_;
	RenderTexture* renderTextures_;
	RenderTexture depthTexture_;
};
