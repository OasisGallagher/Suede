#pragma once
#include "api/gl.h"

/**
 * @param Cull: Front, Back, Off.
 * @param ZTest: Never, Less, LEqual, Equal, Greater, NotEqual, GEqual, Always, Off.
 * @param ZWrite: On, Off.
 * @param Offset0: int(scale Offset::Scale).
 * @param Offset1: int(scale Offset::Scale).
 * @param Blend0: Off, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha
 * @param Blend1: None, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha.
 * @param StencilTest0: Never, Less, LEqual, Equal, Greater, NotEqual, GEqual, Always, Off.
 * @param StencilTest1: [0x00, 0xFF].
 * @param StencilMask: On, Off.
 * @param StencilOp0: Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert.
 * @param StencilOp1: Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert.
 * @param StencilOp2: Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert.
 * @param RasterizerDiscard: On, Off
 */
enum RenderStateType {
	Cull,
	Blend,
	RasterizerDiscard,

	ZTest,
	ZWrite,
	Offset,

	StencilOp,
	StencilTest,
	StencilWrite,

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
	LEqual,
	Equal,
	Greater,
	NotEqual,
	GEqual,
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
	bool IsValidParameter(int value, const int* buffer, int count);
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

class ZTestState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return ZTest; }

private:
	GLenum oldMode_;
	GLboolean oldEnabled_;
	int parameter_;
};

class ZWriteState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return ZWrite; }

private:
	GLint oldMask_;
	int parameter_;
};

class OffsetState : public RenderState {
public:
	enum {
		Scale = 100,
	};

public:
	virtual void Initialize(int parameter0, int parameter1, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return Offset; }

private:
	GLfloat oldUnits_;
	GLfloat oldFactor_;

	int parameter0_;
	int parameter1_;
};

class StencilTestState : public RenderState {
public:
	virtual void Initialize(int parameter0, int parameter1, int);
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
};

class StencilWriteState : public RenderState {
public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return StencilWrite; }

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
