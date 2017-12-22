#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "shader.h"
#include "texture.h"

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

class SUEDE_API IMaterial : virtual public IObject {
public:
	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void SetShader(Shader shader) = 0;
	virtual Shader GetShader() = 0;

	virtual void Define(const std::string& name) = 0;
	virtual void Undefine(const std::string& name) = 0;

	/**
	 * @param Cull: Front, Back, Off.
	 * @param DepthTest: Never, Less, LessEqual, Equal, Greater, NotEqual, GreaterEqual, Always.
	 * @param DepthWrite: On, Off.
	 * @param Blend0: Off, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha
	 * @param Blend1: None, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha.
	 * @param StencilTest0: Never, Less, LessEqual, Equal, Greater, NotEqual, GreaterEqual, Always.
	 * @param StencilTest1: [0x00, 0xFF].
	 * @param StencilTest2: [0x00, 0xFF].
	 * @param StencilMask0: Front, Back, FrontAndBack.
	 * @param StencilMask1: [0x00, 0xFF].
	 * @param StencilOp0: Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert.
	 * @param StencilOp1: Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert.
	 * @param StencilOp2: Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert.
	 * @param RasterizerDiscard: On, Off
	 */
	virtual void SetRenderState(RenderStateType type, int parameter0, int parameter1 = 0, int parameter2 = 0) = 0;

	virtual void SetInt(const std::string& name, int value) = 0;
	virtual void SetFloat(const std::string& name, float value) = 0;
	virtual void SetTexture(const std::string& name, Texture value) = 0;
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value) = 0;
	virtual void SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) = 0;
	virtual void SetVector3(const std::string& name, const glm::vec3& value) = 0;
	virtual void SetVector4(const std::string& name, const glm::vec4& value) = 0;

	virtual int GetInt(const std::string& name) = 0;
	virtual float GetFloat(const std::string& name) = 0;
	virtual Texture GetTexture(const std::string& name) = 0;
	virtual glm::mat4 GetMatrix4(const std::string& name) = 0;
	virtual glm::vec3 GetVector3(const std::string& name) = 0;
	virtual glm::vec4 GetVector4(const std::string& name) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Material);
SUEDE_DECLARE_OBJECT_CREATER(Material);

