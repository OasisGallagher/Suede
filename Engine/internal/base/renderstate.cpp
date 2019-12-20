#include "renderstate.h"

#include <cstdarg>

#include "context.h"
#include "math/mathf.h"
#include "debug/debug.h"
#include "debug/debug.h"
#include "memory/refptr.h"

#define CHECK_PARAMETER(name, value, ...)	\
	if (true) { \
		RenderStateParameter buffer[] = { __VA_ARGS__ }; \
		if (!IsValidParameter(value, buffer, SUEDE_COUNTOF(buffer))) { \
			Debug::LogError("invalid paramter for '%s'", #name); \
		} \
	} else (void)0

void CullState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(RenderStateType::Cull, parameter0, 
		RenderStateParameter::Front, 
		RenderStateParameter::Back, 
		RenderStateParameter::Off
	);

	parameter_ = parameter0;
}

void CullState::Bind() {
	oldEnabled_ = context_->IsEnabled(GL_CULL_FACE);
	context_->GetIntegerv(GL_CULL_FACE_MODE, &oldMode_);

	Enable(GL_CULL_FACE, parameter_ != RenderStateParameter::Off);
	if (parameter_ != RenderStateParameter::Off) {
		context_->CullFace(RenderParamterToGLEnum(parameter_));
	}
}

void CullState::Unbind() {
	Enable(GL_CULL_FACE, oldEnabled_);
	context_->CullFace(oldMode_);
}

RenderState* CullState::Clone() {
	return new CullState(*this);
}

void ZTestState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(RenderStateType::ZTest, parameter0,
		RenderStateParameter::Never, 
		RenderStateParameter::Less, 
		RenderStateParameter::LEqual, 
		RenderStateParameter::Equal, 
		RenderStateParameter::Greater, 
		RenderStateParameter::NotEqual, 
		RenderStateParameter::GEqual, 
		RenderStateParameter::Always, 
		RenderStateParameter::Off
	);

	parameter_ = parameter0;
}

void ZTestState::Bind() {
	oldEnabled_ = context_->IsEnabled(GL_DEPTH_TEST);

	Enable(GL_DEPTH_TEST, parameter_ != RenderStateParameter::Off);
	if (parameter_ != RenderStateParameter::Off) {
		context_->GetIntegerv(GL_DEPTH_FUNC, (int*)&oldMode_);
		context_->DepthFunc(RenderParamterToGLEnum(parameter_));
	}
}

void ZTestState::Unbind() {
	Enable(GL_DEPTH_TEST, oldEnabled_);
	if (parameter_ != RenderStateParameter::Off) {
		context_->DepthFunc(oldMode_);
	}
}

RenderState* ZTestState::Clone() {
	return new ZTestState(*this);
}

void ZWriteState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(RenderStateType::ZWrite, parameter0, RenderStateParameter::On, RenderStateParameter::Off);
	parameter_ = parameter0;
}

void ZWriteState::Bind() {
	context_->GetIntegerv(GL_DEPTH_WRITEMASK, &oldMask_);
	context_->DepthMask(parameter_ == RenderStateParameter::On);
}

void ZWriteState::Unbind() {
	context_->DepthMask(!!oldMask_);
}

RenderState* ZWriteState::Clone() {
	return new ZWriteState(*this);
}

void OffsetState::Initialize(int parameter0, int parameter1, int) {
	parameter0_ = parameter0;
	parameter1_ = parameter1;
}

void OffsetState::Bind() {
	if (parameter0_ != 0 || parameter1_ != 0) {
		oldEnabled_ = context_->IsEnabled(GL_POLYGON_OFFSET_FILL);

		context_->GetFloatv(GL_POLYGON_OFFSET_UNITS, &oldUnits_);
		context_->GetFloatv(GL_POLYGON_OFFSET_FACTOR, &oldFactor_);

		context_->Enable(GL_POLYGON_OFFSET_FILL);
		context_->PolygonOffset(float(parameter0_) / Scale, float(parameter1_) / Scale);
	}
}

void OffsetState::Unbind() {
	if (parameter0_ != 0 || parameter1_ != 0) {
		context_->PolygonOffset(oldFactor_, oldUnits_);
		Enable(GL_POLYGON_OFFSET_FILL, oldEnabled_);
	}
}

RenderState* OffsetState::Clone() {
	return new OffsetState(*this);
}

void StencilTestState::Initialize(int parameter0, int parameter1, int parameter2) {
	CHECK_PARAMETER(RenderStateType::StencilTest, parameter0,
		RenderStateParameter::Never, 
		RenderStateParameter::Less, 
		RenderStateParameter::LEqual, 
		RenderStateParameter::Equal, 
		RenderStateParameter::Greater, 
		RenderStateParameter::NotEqual, 
		RenderStateParameter::GEqual, 
		RenderStateParameter::Always, 
		RenderStateParameter::Off
	);

	if (parameter0 != RenderStateParameter::Off && (parameter1 > 0xFF || parameter1 < 0x00)) {
		Debug::LogError("invalid parameter1 for 'StencilTest'.");
		return;
	}

	parameter0_ = parameter0;
	parameter1_ = parameter1;
}

void StencilTestState::Bind() {
	oldEnabled_ = context_->IsEnabled(GL_STENCIL_TEST);

	Enable(GL_STENCIL_TEST, parameter0_ != RenderStateParameter::Off);
	if (parameter0_ != RenderStateParameter::Off) {
		context_->GetIntegerv(GL_STENCIL_REF, (int*)&oldRef_);
		context_->GetIntegerv(GL_STENCIL_FUNC, (int*)&oldFunc_);
		context_->GetIntegerv(GL_STENCIL_VALUE_MASK, (int*)&oldMask_);

		context_->StencilFunc(RenderParamterToGLEnum(parameter0_), parameter1_, 0xFF);
	}
}

void StencilTestState::Unbind() {
	Enable(GL_STENCIL_TEST, oldEnabled_);
	if (parameter0_ != RenderStateParameter::Off) {
		context_->StencilFunc(oldFunc_, oldRef_, oldMask_);
	}
}

RenderState* StencilTestState::Clone() {
	return new StencilTestState(*this);
}

void StencilWriteState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(RenderStateType::StencilWrite, parameter0, RenderStateParameter::On, RenderStateParameter::Off);

	parameter0_ = parameter0;
}

void StencilWriteState::Bind() {
	context_->GetIntegerv(GL_STENCIL_WRITEMASK, (int*)&oldFrontMask_);
	context_->GetIntegerv(GL_STENCIL_BACK_WRITEMASK, (int*)&oldBackMask_);

	context_->StencilMask(parameter0_ == RenderStateParameter::On ? 0xFF : 0);
}

void StencilWriteState::Unbind() {
	context_->StencilMaskSeparate(GL_FRONT, oldFrontMask_);
	context_->StencilMaskSeparate(GL_BACK, oldBackMask_);
}

RenderState* StencilWriteState::Clone() {
	return new StencilWriteState(*this);
}

void StencilOpState::Initialize(int parameter0, int parameter1, int parameter2) {
	CHECK_PARAMETER(RenderStateType::StencilOp, parameter0,
		RenderStateParameter::Keep, 
		RenderStateParameter::Zero, 
		RenderStateParameter::Replace, 
		RenderStateParameter::Incr, 
		RenderStateParameter::IncrWrap, 
		RenderStateParameter::Decr, 
		RenderStateParameter::DecrWrap, 
		RenderStateParameter::Invert);

	CHECK_PARAMETER(RenderStateType::StencilOp, parameter1,
		RenderStateParameter::Keep, 
		RenderStateParameter::Zero, 
		RenderStateParameter::Replace, 
		RenderStateParameter::Incr, 
		RenderStateParameter::IncrWrap, 
		RenderStateParameter::Decr, 
		RenderStateParameter::DecrWrap, 
		RenderStateParameter::Invert
	);

	CHECK_PARAMETER(RenderStateType::StencilOp, parameter2,
		RenderStateParameter::Keep, 
		RenderStateParameter::Zero, 
		RenderStateParameter::Replace, 
		RenderStateParameter::Incr, 
		RenderStateParameter::IncrWrap, 
		RenderStateParameter::Decr, 
		RenderStateParameter::DecrWrap, 
		RenderStateParameter::Invert
	);

	parameter0_ = parameter0;
	parameter1_ = parameter1;
	parameter2_ = parameter2;
}

void StencilOpState::Bind() {
	context_->GetIntegerv(GL_STENCIL_FAIL, (int*)&oldSfail_);
	context_->GetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, (int*)&oldDpfail_);
	context_->GetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, (int*)&oldDppass_);

	context_->StencilOp(RenderParamterToGLEnum(parameter0_), RenderParamterToGLEnum(parameter1_), RenderParamterToGLEnum(parameter2_));
}

void StencilOpState::Unbind() {
	context_->StencilOp(oldSfail_, oldDpfail_, oldDppass_);
}

RenderState* StencilOpState::Clone() {
	return new StencilOpState(*this);
}

void RasterizerDiscardState::Initialize(int parameter0, int, int) {
	CHECK_PARAMETER(RenderStateType::RasterizerDiscard, parameter0, RenderStateParameter::On, RenderStateParameter::Off);
	parameter_ = parameter0;
}

void RasterizerDiscardState::Bind() {
	oldEnabled_ = context_->IsEnabled(GL_RASTERIZER_DISCARD);
	Enable(GL_RASTERIZER_DISCARD, parameter_ == RenderStateParameter::On);
}

void RasterizerDiscardState::Unbind() {
	Enable(GL_RASTERIZER_DISCARD, oldEnabled_);
}

RenderState* RasterizerDiscardState::Clone() {
	return new RasterizerDiscardState(*this);
}

void BlendState::Initialize(int parameter0, int parameter1, int) {
	CHECK_PARAMETER(RenderStateType::Blend, parameter0,
		RenderStateParameter::Off, 
		RenderStateParameter::Zero, 
		RenderStateParameter::One, 
		RenderStateParameter::SrcColor, 
		RenderStateParameter::OneMinusSrcColor, 
		RenderStateParameter::SrcAlpha, 
		RenderStateParameter::OneMinusSrcAlpha, 
		RenderStateParameter::DestAlpha, 
		RenderStateParameter::OneMinusDestAlpha
	);

	if (parameter0 != RenderStateParameter::Off) {
		CHECK_PARAMETER(RenderStateType::Blend, parameter0,
			RenderStateParameter::None, 
			RenderStateParameter::Zero, 
			RenderStateParameter::One, 
			RenderStateParameter::SrcColor, 
			RenderStateParameter::OneMinusSrcColor, 
			RenderStateParameter::SrcAlpha, 
			RenderStateParameter::OneMinusSrcAlpha, 
			RenderStateParameter::DestAlpha, 
			RenderStateParameter::OneMinusDestAlpha
		);
	}

	src_ = parameter0;
	dest_ = parameter1;
}

void BlendState::Bind() {
	oldEnabled_ = context_->IsEnabled(GL_BLEND);
	context_->GetIntegerv(GL_BLEND_SRC, &oldSrc_);
	context_->GetIntegerv(GL_BLEND_DST, &oldDest_);

	Enable(GL_BLEND, src_ != RenderStateParameter::Off);
	if (src_ != RenderStateParameter::Off) {
		context_->BlendFunc(RenderParamterToGLEnum(src_), RenderParamterToGLEnum(dest_));
	}
}

void BlendState::Unbind() {
	Enable(GL_BLEND, oldEnabled_);
	context_->BlendFunc(oldSrc_, oldDest_);
}

RenderState* BlendState::Clone() {
	return new BlendState(*this);
}

void RenderState::Enable(uint cap, bool enable) {
	if (enable) { context_->Enable(cap); }
	else { context_->Disable(cap); }
}

bool RenderState::IsValidParameter(int value, const RenderStateParameter* buffer, int count) {
	int i = 0;
	for (; i < count; ++i) {
		if (value == buffer[i]) {
			break;
		}
	}

	return (i < count);
}

uint RenderState::RenderParamterToGLEnum(int parameter0) {
	uint value = 0;
	switch (parameter0) {
		case RenderStateParameter::Front:
			value = GL_FRONT;
			break;
		case RenderStateParameter::Back:
			value = GL_BACK;
			break;
		case RenderStateParameter::FrontAndBack:
			value = GL_FRONT_AND_BACK;
			break;
		case RenderStateParameter::Never:
			value = GL_NEVER;
			break;
		case RenderStateParameter::Less:
			value = GL_LESS;
			break;
		case RenderStateParameter::LEqual:
			value = GL_LEQUAL;
			break;
		case RenderStateParameter::Equal:
			value = GL_EQUAL;
			break;
		case RenderStateParameter::Greater:
			value = GL_GREATER;
			break;
		case RenderStateParameter::NotEqual:
			value = GL_NOTEQUAL;
			break;
		case RenderStateParameter::GEqual:
			value = GL_GEQUAL;
			break;
		case RenderStateParameter::Always:
			value = GL_ALWAYS;
			break;
		case RenderStateParameter::Zero:
			value = GL_ZERO;
			break;
		case RenderStateParameter::One:
			value = GL_ONE;
			break;
		case RenderStateParameter::SrcColor:
			value = GL_SRC_COLOR;
			break;
		case RenderStateParameter::OneMinusSrcColor:
			value = GL_ONE_MINUS_SRC_COLOR;
			break;
		case RenderStateParameter::SrcAlpha:
			value = GL_SRC_ALPHA;
			break;
		case RenderStateParameter::OneMinusSrcAlpha:
			value = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case RenderStateParameter::DestAlpha:
			value = GL_DST_ALPHA;
			break;
		case RenderStateParameter::OneMinusDestAlpha:
			value = GL_ONE_MINUS_DST_ALPHA;
			break;
		case RenderStateParameter::Keep:
			value = GL_KEEP;
			break;
		case RenderStateParameter::Replace:
			value = GL_REPLACE;
			break;
		case RenderStateParameter::Incr:
			value = GL_INCR;
			break;
		case RenderStateParameter::IncrWrap:
			value = GL_INCR_WRAP;
			break;
		case RenderStateParameter::Decr:
			value = GL_DECR;
			break;
		case RenderStateParameter::DecrWrap:
			value = GL_DECR_WRAP;
			break;
		case RenderStateParameter::Invert:
			value = GL_INVERT;
			break;
	}

	if (value == 0) {
		Debug::LogError("invalid render paramter %d.", parameter0);
	}

	return value;
}
