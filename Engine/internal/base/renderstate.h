#pragma once
#include <wrappers/gl.h>

#include "renderer.h"

class RenderState {
public:
	virtual ~RenderState() {}
	virtual void Initialize(int parameter0, int parameter1, int parameter2) = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual RenderState* Clone() = 0;

protected:
	void Enable(GLenum cap, GLboolean enable);
	GLenum RenderParamterToGLEnum(int parameter);
	bool IsValidParamter(int parameter, int count, ...);
};

class CullState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	int oldMode_;
	GLboolean oldEnabled_;
	int parameter_;
};

class DepthTestState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLenum oldMode_;
	GLboolean oldEnabled_;
	int parameter_;
};

class DepthWriteState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLint oldMask_;
	int parameter_;
};

class StencilTestState : public RenderState {
public:
	virtual void Initialize(int parameter0, int parameter1, int parameter2);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLint oldRef_;
	GLenum oldFunc_;
	GLuint oldMask_;

	GLboolean oldEnabled_;

	int parameter0_;
	int parameter1_;
	int parameter2_;
};

class StencilMaskState : public RenderState {
public:
	virtual void Initialize(int parameter0, int parameter1, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLuint oldBackMask_;
	GLuint oldFrontMask_;

	int parameter0_;
	int parameter1_;
};

class StencilOpState : public RenderState {
public:
	virtual void Initialize(int parameter0, int parameter1, int parameter2);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLenum oldSfail_;
	GLenum oldDpfail_;
	GLenum oldDppass_;

	int parameter0_;
	int parameter1_;
	int parameter2_;
};

class RasterizerDiscardState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLboolean oldEnabled_;
	int parameter_;
};

class BlendState : public RenderState {
public:
	virtual void Initialize(int parameter0, int parameter1, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();

private:
	GLboolean oldEnabled_;
	int oldSrc_, oldDest_;
	int src_, dest_;
};
