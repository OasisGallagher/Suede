#include <cstdarg>
#include "tools/math2.h"
#include "debug/debug.h"
#include "renderstate.h"
#include "debug/debug.h"
#include "memory/memory.h"

#define CHECK_PARAMETER(name, value, ...)	\
	if (true) { \
		int buffer[] = { __VA_ARGS__ }; \
		if (!IsValidParameter(value, buffer, CountOf(buffer))) { \
			Debug::LogError("invalid paramter for '%s'", #name); \
		} \
	} else (void)0

void CullState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(Cull, parameter0, Back, Off);
	parameter_ = parameter0;
}

void CullState::Bind() {
	oldEnabled_ = GL::IsEnabled(GL_CULL_FACE);
	GL::GetIntegerv(GL_CULL_FACE_MODE, &oldMode_);

	Enable(GL_CULL_FACE, parameter_ != Off);
	if (parameter_ != Off) {
		GL::CullFace(RenderParamterToGLEnum(parameter_));
	}
}

void CullState::Unbind() {
	Enable(GL_CULL_FACE, oldEnabled_);
	GL::CullFace(oldMode_);
}

RenderState * CullState::Clone() {
	return MEMORY_CLONE(CullState, this);
}

void ZTestState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(ZTest, parameter0, Never, Less, LEqual, Equal, Greater, NotEqual, GEqual, Always, Off);
	parameter_ = parameter0;
}

void ZTestState::Bind() {
	oldEnabled_ = GL::IsEnabled(GL_DEPTH_TEST);

	Enable(GL_DEPTH_TEST, parameter_ != Off);
	if (parameter_ != Off) {
		GL::GetIntegerv(GL_DEPTH_FUNC, (GLint*)&oldMode_);
		GL::DepthFunc(RenderParamterToGLEnum(parameter_));
	}
}

void ZTestState::Unbind() {
	Enable(GL_DEPTH_TEST, oldEnabled_);
	if (parameter_ != Off) {
		GL::DepthFunc(oldMode_);
	}
}

RenderState * ZTestState::Clone() {
	return MEMORY_CLONE(ZTestState, this);
}

void ZWriteState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(ZWrite, parameter0, On, Off);
	parameter_ = parameter0;
}

void ZWriteState::Bind() {
	GL::GetIntegerv(GL_DEPTH_WRITEMASK, &oldMask_);
	GL::DepthMask(parameter_ == On);
}

void ZWriteState::Unbind() {
	GL::DepthMask(oldMask_);
}

RenderState* ZWriteState::Clone() {
	return MEMORY_CLONE(ZWriteState, this);
}

void StencilTestState::Initialize(int parameter0, int parameter1, int parameter2) {
	CHECK_PARAMETER(StencilTest, parameter0, Never, Less, LEqual, Equal, Greater, NotEqual, GEqual, Always, Off);

	if (parameter1 > 0xFF || parameter1 < 0x00) {
		Debug::LogError("invalid parameter1 for 'StencilTest'.");
		return;
	}

	if (parameter2 > 0xFF || parameter2 < 0x00) {
		Debug::LogError("invalid parameter2 for 'StencilTest'.");
		return;
	}

	parameter0_ = parameter0;
	parameter1_ = parameter1;
	parameter2_ = parameter2;
}

void StencilTestState::Bind() {
	oldEnabled_ = GL::IsEnabled(GL_STENCIL_TEST);

	Enable(GL_STENCIL_TEST, parameter0_ != Off);
	if (parameter0_ != Off) {
		GL::GetIntegerv(GL_STENCIL_REF, (GLint*)&oldRef_);
		GL::GetIntegerv(GL_STENCIL_FUNC, (GLint*)&oldFunc_);
		GL::GetIntegerv(GL_STENCIL_VALUE_MASK, (GLint*)&oldMask_);

		GL::StencilFunc(RenderParamterToGLEnum(parameter0_), parameter1_, parameter2_);
	}
}

void StencilTestState::Unbind() {
	Enable(GL_STENCIL_TEST, oldEnabled_);
	if (parameter0_ != Off) {
		GL::StencilFunc(oldFunc_, oldRef_, oldMask_);
	}
}

RenderState * StencilTestState::Clone() {
	return MEMORY_CLONE(StencilTestState, this);
}

void StencilMaskState::Initialize(int parameter0, int parameter1, int) {
	CHECK_PARAMETER(StencilMask, parameter0, Front, Back, FrontAndBack);

	if (parameter1 > 0xFF || parameter1 < 0x00) {
		Debug::LogError("invalid parameter1 for 'StencilMask'.");
		return;
	}

	parameter0_ = parameter0;
	parameter1_ = parameter1;
}

void StencilMaskState::Bind() {
	GL::GetIntegerv(GL_STENCIL_WRITEMASK, (GLint*)&oldFrontMask_);
	GL::GetIntegerv(GL_STENCIL_BACK_WRITEMASK, (GLint*)&oldBackMask_);

	GL::StencilMaskSeparate(RenderParamterToGLEnum(parameter0_), parameter1_);
}

void StencilMaskState::Unbind() {
	GL::StencilMaskSeparate(GL_FRONT, oldFrontMask_);
	GL::StencilMaskSeparate(GL_BACK, oldBackMask_);
}

RenderState * StencilMaskState::Clone() {
	return MEMORY_CLONE(StencilMaskState, this);
}

void StencilOpState::Initialize(int parameter0, int parameter1, int parameter2) {
	CHECK_PARAMETER(StencilOp, parameter0, Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert);
	CHECK_PARAMETER(StencilOp, parameter1, Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert);
	CHECK_PARAMETER(StencilOp, parameter2, Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert);

	parameter0_ = parameter0;
	parameter1_ = parameter1;
	parameter2_ = parameter2;
}

void StencilOpState::Bind() {
	GL::GetIntegerv(GL_STENCIL_FAIL, (GLint*)&oldSfail_);
	GL::GetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, (GLint*)&oldDpfail_);
	GL::GetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, (GLint*)&oldDppass_);

	GL::StencilOp(RenderParamterToGLEnum(parameter0_), RenderParamterToGLEnum(parameter1_), RenderParamterToGLEnum(parameter2_));
}

void StencilOpState::Unbind() {
	GL::StencilOp(oldSfail_, oldDpfail_, oldDppass_);
}

RenderState * StencilOpState::Clone() {
	return MEMORY_CLONE(StencilOpState, this);
}

void RasterizerDiscardState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(RasterizerDiscard, parameter0, On, Off);
	parameter_ = parameter0;
}

void RasterizerDiscardState::Bind() {
	oldEnabled_ = GL::IsEnabled(GL_RASTERIZER_DISCARD);
	Enable(GL_RASTERIZER_DISCARD, parameter_ == On);
}

void RasterizerDiscardState::Unbind() {
	Enable(GL_RASTERIZER_DISCARD, oldEnabled_);
}

RenderState * RasterizerDiscardState::Clone() {
	return MEMORY_CLONE(RasterizerDiscardState, this);
}

void BlendState::Initialize(int parameter0, int parameter1, int) {
	CHECK_PARAMETER(Blend, parameter0, Off, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha);

	if (parameter0 != Off) {
		CHECK_PARAMETER(Blend, parameter0, None, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha);
	}

	src_ = parameter0;
	dest_ = parameter1;
}

void BlendState::Bind() {
	oldEnabled_ = GL::IsEnabled(GL_BLEND);
	GL::GetIntegerv(GL_BLEND_SRC, &oldSrc_);
	GL::GetIntegerv(GL_BLEND_DST, &oldDest_);

	Enable(GL_BLEND, src_ != Off);
	if (src_ != Off) {
		GL::BlendFunc(RenderParamterToGLEnum(src_), RenderParamterToGLEnum(dest_));
	}
}

void BlendState::Unbind() {
	Enable(GL_BLEND, oldEnabled_);
	GL::BlendFunc(oldSrc_, oldDest_);
}

RenderState * BlendState::Clone() {
	return MEMORY_CLONE(BlendState, this);
}

void RenderState::Enable(GLenum cap, GLboolean enable) {
	if (enable) { GL::Enable(cap); }
	else { GL::Disable(cap); }
}

bool RenderState::IsValidParameter(int value, const int* buffer, int count) {
	int i = 0;
	for (; i < count; ++i) {
		if (value == buffer[i]) {
			break;
		}
	}

	return (i < count);
}

GLenum RenderState::RenderParamterToGLEnum(int parameter0) {
	GLenum value = 0;
	switch (parameter0) {
		case Front:
			value = GL_FRONT;
			break;
		case Back:
			value = GL_BACK;
			break;
		case FrontAndBack:
			value = GL_FRONT_AND_BACK;
			break;
		case Never:
			value = GL_NEVER;
			break;
		case Less:
			value = GL_LESS;
			break;
		case LEqual:
			value = GL_LEQUAL;
			break;
		case Equal:
			value = GL_EQUAL;
			break;
		case Greater:
			value = GL_GREATER;
			break;
		case NotEqual:
			value = GL_NOTEQUAL;
			break;
		case GEqual:
			value = GL_GEQUAL;
			break;
		case Always:
			value = GL_ALWAYS;
			break;
		case Zero:
			value = GL_ZERO;
			break;
		case One:
			value = GL_ONE;
			break;
		case SrcColor:
			value = GL_SRC_COLOR;
			break;
		case OneMinusSrcColor:
			value = GL_ONE_MINUS_SRC_COLOR;
			break;
		case SrcAlpha:
			value = GL_SRC_ALPHA;
			break;
		case OneMinusSrcAlpha:
			value = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case DestAlpha:
			value = GL_DST_ALPHA;
			break;
		case OneMinusDestAlpha:
			value = GL_ONE_MINUS_DST_ALPHA;
			break;
		case Keep:
			value = GL_KEEP;
			break;
		case Replace:
			value = GL_REPLACE;
			break;
		case Incr:
			value = GL_INCR;
			break;
		case IncrWrap:
			value = GL_INCR_WRAP;
			break;
		case Decr:
			value = GL_DECR;
			break;
		case DecrWrap:
			value = GL_DECR_WRAP;
			break;
		case Invert:
			value = GL_INVERT;
			break;
	}

	if (value == 0) {
		Debug::LogError("invalid render paramter %d.", parameter0);
	}

	return value;
}
