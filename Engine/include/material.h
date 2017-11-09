#pragma once
#include <glm/glm.hpp>

#include "object.h"
#include "shader.h"
#include "texture.h"

enum RenderStateType {
	Cull,
	Blend,
	DepthTest,
	DepthWrite,
	RasterizerDiscard,

	RenderStateCount,
};

enum {
	None,
	Front,
	Back,
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
};

class ENGINE_EXPORT IMaterial : virtual public IObject {
public:
	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void SetShader(Shader shader) = 0;
	virtual Shader GetShader() = 0;

	virtual void Define(const std::string& name) = 0;
	virtual void Undefine(const std::string& name) = 0;

	/**
	* @param Cull: Front, Back, Off
	* @param DepthTest: Never, Less, LessEqual, Equal, Greater, NotEqual, GreaterEqual, Always
	* @param DepthWrite: On, Off
	* @param Blend0: Off, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha
	* @param Blend1: None, Zero, One, SrcColor, OneMinusSrcColor, SrcAlpha, OneMinusSrcAlpha, DestAlpha, OneMinusDestAlpha
	* @param RasterizerDiscard: On, Off
	*/
	virtual void SetRenderState(RenderStateType type, int parameter0, int parameter1 = 0) = 0;

	virtual void SetInt(const std::string& name, int value) = 0;
	virtual void SetFloat(const std::string& name, float value) = 0;
	virtual void SetTexture(const std::string& name, Texture value) = 0;
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value) = 0;
	virtual void SetVector3(const std::string& name, const glm::vec3& value) = 0;

	virtual int GetInt(const std::string& name) = 0;
	virtual float GetFloat(const std::string& name) = 0;
	virtual Texture GetTexture(const std::string& name) = 0;
	virtual glm::mat4 GetMatrix4(const std::string& name) = 0;
	virtual glm::vec3 GetVector3(const std::string& name) = 0;
};

typedef std::shared_ptr<IMaterial> Material;
