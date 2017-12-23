#include <cstdarg>
#include "renderstate.h"
#include "debug/debug.h"
#include "internal/memory/memory.h"

void CullState::Initialize(int parameter0, int, int) {
	if (!IsValidParamter(parameter0, 3,
		Front, Back, Off)) {
		Debug::LogError("invalid paramter for 'Cull'.");
		return;
	}

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

void DepthTestState::Initialize(int parameter0, int, int) {
	if (!IsValidParamter(parameter0, 8,
		Never, Less, LessEqual, Equal, Greater, NotEqual, GreaterEqual, Always)) {
		Debug::LogError("invalid parameter0 for 'DepthTest'.");
		return;
	}

	parameter_ = parameter0;
}

void DepthTestState::Bind() {
	oldEnabled_ = GL::IsEnabled(GL_DEPTH_TEST);
	GL::GetIntegerv(GL_DEPTH_FUNC, (GLint*)&oldMode_);

	Enable(GL_DEPTH_TEST, parameter_ != Always);
	GL::DepthFunc(RenderParamterToGLEnum(parameter_));
}

void DepthTestState::Unbind() {
	Enable(GL_DEPTH_TEST, oldEnabled_);
	GL::DepthFunc(oldMode_);
}

RenderState * DepthTestState::Clone() {
	return MEMORY_CLONE(DepthTestState, this);
}

void DepthWriteState::Initialize(int parameter0, int, int) {
	if (!IsValidParamter(parameter0, 2,
		On, Off)) {
		Debug::LogError("invalid paramter for 'DepthWrite'.");
		return;
	}

	parameter_ = parameter0;
}

void DepthWriteState::Bind() {
	GL::GetIntegerv(GL_DEPTH_WRITEMASK, &oldMask_);
	GL::DepthMask(parameter_ == On);
}

void DepthWriteState::Unbind() {
	GL::DepthMask(oldMask_);
}

RenderState * DepthWriteState::Clone() {
	return MEMORY_CLONE(DepthWriteState, this);
}

void StencilTestState::Initialize(int parameter0, int parameter1, int parameter2) {
	if (!IsValidParamter(parameter0, 8,
		Never, Less, LessEqual, Equal, Greater, NotEqual, GreaterEqual, Always)) {
		Debug::LogError("invalid parameter0 for 'StencilTest'.");
		return;
	}

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

	GL::GetIntegerv(GL_STENCIL_REF, (GLint*)&oldRef_);
	GL::GetIntegerv(GL_STENCIL_FUNC, (GLint*)&oldFunc_);
	GL::GetIntegerv(GL_STENCIL_VALUE_MASK, (GLint*)&oldMask_);

	Enable(GL_STENCIL_TEST, parameter0_ != Always);
	GL::StencilFunc(RenderParamterToGLEnum(parameter0_), parameter1_, parameter2_);
}

void StencilTestState::Unbind() {
	Enable(GL_STENCIL_TEST, oldEnabled_);
	GL::StencilFunc(oldRef_, oldFunc_, oldMask_);
}

RenderState * StencilTestState::Clone() {
	return MEMORY_CLONE(StencilTestState, this);
}

void StencilMaskState::Initialize(int parameter0, int parameter1, int) {
	if (!IsValidParamter(parameter0, 3,
		Front, Back, FrontAndBack)) {
		Debug::LogError("invalid parameter0 for 'StencilMask'.");
		return;
	}

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
	if (!IsValidParamter(parameter0, 8,
		Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert)) {
		Debug::LogError("invalid parameter0 for 'StencilOp'.");
		return;
	}

	if (!IsValidParamter(parameter1, 8,
		Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert)) {
		Debug::LogError("invalid parameter1 for 'StencilOp'.");
		return;
	}

	if (!IsValidParamter(parameter2, 8,
		Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert)) {
		Debug::LogError("invalid parameter2 for 'StencilOp'.");
		return;
	}

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
	if (!IsValidParamter(parameter0, 2,
		On, Off)) {
		Debug::LogError("invalid paramter for 'RasterizerDiscard'.");
		return;
	}

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
	if (!IsValidParamter(parameter0, 9,
		Off, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha)) {
		Debug::LogError("invalid paramter for 'Blend'.");
		return;
	}

	if (parameter0 != Off && !IsValidParamter(parameter1, 9,
		None, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha)) {
		Debug::LogError("invalid paramter for 'Blend'.");
		return;
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

bool RenderState::IsValidParamter(int parameter0, int count, ...) {
	va_list vl;
	va_start(vl, count);

	int i = 0;
	for (; i < count; ++i) {
		if (parameter0 == va_arg(vl, int)) {
			break;
		}
	}

	va_end(vl);

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
		case LessEqual:
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
		case GreaterEqual:
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
