#pragma once

#include "material.h"
#include "internal/containers/ptrmap.h"
#include "internal/containers/variant.h"
#include "internal/base/objectinternal.h"

enum VertexAttributeIndex {
	VertexAttributeIndexPosition,
	VertexAttributeIndexTexCoord,
	VertexAttributeIndexNormal,
	VertexAttributeIndexTangent,
	VertexAttributeIndexBoneIndexes,
	VertexAttributeIndexBoneWeights,

	VertexAttributeIndexUser0,
	VertexAttributeIndexUser1,
	VertexAttributeIndexUser2,
};

class MaterialInternal : public IMaterial, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Material)

public:
	MaterialInternal();
	~MaterialInternal() {}

public:
	virtual void SetShader(Shader value);
	virtual Shader GetShader() { return shader_; }

	virtual void Bind();
	virtual void Unbind();

	virtual void Define(const std::string& name);
	virtual void Undefine(const std::string& name);

	virtual void SetInt(const std::string& name, int value);
	virtual void SetFloat(const std::string& name, float value);
	virtual void SetTexture(const std::string& name, Texture value);
	virtual void SetMatrix4(const std::string& name, const glm::mat4& value);
	virtual void SetVector3(const std::string& name, const glm::vec3& value);

	virtual int GetInt(const std::string& name);
	virtual float GetFloat(const std::string& name);
	virtual Texture GetTexture(const std::string& name);
	virtual glm::mat4 GetMatrix4(const std::string& name);
	virtual glm::vec3 GetVector3(const std::string& name);

private:
	struct Uniform {
		GLenum type;
		union { GLuint offset, location; };
		GLuint size;
		Variant value;
	};

	typedef PtrMap<Uniform> UniformContainer;

private:
	bool IsSampler(int type);

	void UpdateVariables();
	void UpdateVertexAttributes();
	void UpdateFragmentAttributes();
	
	void AddAllUniforms();
	void AddUniform(const char* name, GLenum type, GLuint location, GLint size);

	void BindTextures();
	void UnbindTextures();
	
	GLuint GetSizeOfType(GLint type);
	GLuint GetUniformSize(GLint uniformType, GLint uniformSize, GLint uniformOffset, GLint uniformMatrixStride, GLint uniformArrayStride);

	Uniform* GetUniform(const std::string& name, VariantType type);

	void SetUniform(struct Uniform* u, const void* value);

private:
	Shader shader_;
	Texture diffuse_;
	int oldProgram_;
	int maxTextureUnits_;
	int textureUnitIndex_;
	UniformContainer uniforms_;
};
