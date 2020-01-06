#pragma once
#include "types.h"
#include "tools/enum.h"

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
BETTER_ENUM(RenderStateType, int,
	Cull,
	Blend,
	RasterizerDiscard,

	ZTest,
	ZWrite,
	Offset,

	StencilOp,
	StencilTest,
	StencilWrite
)

BETTER_ENUM(RenderStateParameter, int,
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
	Invert
)

class Context;
class RenderState {
public:
	RenderState(Context* context) : context_(context) {}
	virtual ~RenderState() {}

public:
	virtual void Initialize(int parameter0, int parameter1, int parameter2) = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual RenderState* Clone() = 0;
	virtual RenderStateType GetType() const = 0;

protected:
	void Enable(uint cap, bool enable);
	uint RenderParamterToGLEnum(int parameter);
	bool IsValidParameter(int value, const RenderStateParameter* buffer, int count);

protected:
	Context* context_;
};

class CullState : public RenderState {
public:
	CullState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::Cull; }

private:
	int oldMode_;
	bool oldEnabled_;
	int parameter_;
};

class ZTestState : public RenderState {
public:
	ZTestState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::ZTest; }

private:
	uint oldMode_;
	bool oldEnabled_;
	int parameter_;
};

class ZWriteState : public RenderState {
public:
	ZWriteState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::ZWrite; }

private:
	int oldMask_;
	int parameter_;
};

class OffsetState : public RenderState {
public:
	OffsetState(Context* context) : RenderState(context) {}

public:
	enum {
		Scale = 100,
	};

public:
	virtual void Initialize(int parameter0, int parameter1, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::Offset; }

private:
	float oldUnits_;
	float oldFactor_;
	bool oldEnabled_;

	int parameter0_;
	int parameter1_;
};

class StencilTestState : public RenderState {
public:
	StencilTestState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int parameter1, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::StencilTest; }

private:
	int oldRef_;
	uint oldFunc_;
	uint oldMask_;

	bool oldEnabled_;

	int parameter0_;
	int parameter1_;
};

class StencilWriteState : public RenderState {
public:
	StencilWriteState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::StencilWrite; }

private:
	uint oldBackMask_;
	uint oldFrontMask_;

	int parameter0_;
	int parameter1_;
};

class StencilOpState : public RenderState {
public:
	StencilOpState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int parameter1, int parameter2);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::StencilOp; }

private:
	uint oldSfail_;
	uint oldDpfail_;
	uint oldDppass_;

	int parameter0_;
	int parameter1_;
	int parameter2_;
};

class RasterizerDiscardState : public RenderState {
public:
	RasterizerDiscardState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::RasterizerDiscard; }

private:
	bool oldEnabled_;
	int parameter_;
};

class BlendState : public RenderState {
public:
	BlendState(Context* context) : RenderState(context) {}

public:
	virtual void Initialize(int parameter0, int parameter1, int);
	virtual void Bind();
	virtual void Unbind();
	virtual RenderState* Clone();
	virtual RenderStateType GetType() const { return RenderStateType::Blend; }

private:
	bool oldEnabled_;
	int oldSrc_, oldDest_;
	int src_, dest_;
};
