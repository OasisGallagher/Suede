#pragma once
#include <wrappers/gl.h>

#include "renderer.h"

enum RenderStateType {
	Cull,
	Blend,
	RasterizerDiscard,

	DepthTest,
	DepthWrite,

	StencilOp,
	StencilTest,
	StencilMask,

	RenderStateCount,
};

class RenderStateParameter {
	struct Parameter {
		int value;
		const char* text;
	};
};

enum {
	None,
	Front,
	Back,
	FrontAndBack,

	On,
	Off,

	Never,
	Less,
	LessEqual,
	Equal,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,

	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DestAlpha,
	OneMinusDestAlpha,

	Keep,
	Replace,
	Incr,
	IncrWrap,
	Decr,
	DecrWrap,
	Invert,
};

class RenderState {
public:
	virtual ~RenderState() {}
	virtual void Initialize(int parameter0, int parameter1, int parameter2) = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual RenderState* Clone() = 0;
	virtual RenderStateType GetType() const = 0;

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
	virtual RenderStateType GetType() const { return Cull; }
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
	virtual RenderStateType GetType() const { return DepthTest; }

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
	virtual RenderStateType GetType() const { return DepthWrite; }

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
	virtual RenderStateType GetType() const { return StencilTest; }

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
	virtual RenderStateType GetType() const { return StencilMask; }

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
	virtual RenderStateType GetType() const { return StencilOp; }

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
	virtual RenderStateType GetType() const { return RasterizerDiscard; }

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
	virtual RenderStateType GetType() const { return Blend; }

private:
	GLboolean oldEnabled_;
	int oldSrc_, oldDest_;
	int src_, dest_;
};
