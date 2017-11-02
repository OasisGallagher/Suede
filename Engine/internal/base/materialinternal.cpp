#include "variables.h"
#include "tools/mathf.h"
#include "tools/string.h"
#include "materialinternal.h"

MaterialInternal::MaterialInternal()
	: ObjectInternal(ObjectTypeMaterial)
	, textureUnitIndex_(0), maxTextureUnits_(0), oldProgram_(0) {
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits_);
}

void MaterialInternal::SetShader(Shader value) {
	shader_ = value;
	
	UnbindTextures();

	UpdateVertexAttributes();
	UpdateFragmentAttributes();

	shader_->Link();

	UpdateVariables();
}

void MaterialInternal::SetInt(const std::string& name, int value) {
	Uniform* u = GetUniform(name, VariantTypeInt);
	if (u != nullptr && u->value.GetInt() != value) {
		glProgramUniform1i(shader_->GetNativePointer(), u->location, value);
	}
}

void MaterialInternal::SetFloat(const std::string& name, float value) {
	Uniform* u = GetUniform(name, VariantTypeFloat);
	if (u != nullptr && !Mathf::Approximately(u->value.GetFloat(), value)) {
		glProgramUniform1f(shader_->GetNativePointer(), u->location, value);
	}
}

void MaterialInternal::SetTexture(const std::string& name, Texture value) {
	Uniform* u = GetUniform(name, VariantTypeTexture);

	if (u != nullptr && u->value.GetTexture() != value) {
		u->value.SetTexture(value);
		glProgramUniform1i(shader_->GetNativePointer(), u->location, u->value.GetTextureIndex());
	}
}

void MaterialInternal::SetVector3(const std::string& name, const glm::vec3& value) {
	Uniform* u = GetUniform(name, VariantTypeVector3);
	if (u != nullptr && u->value.GetVector3() != value) {
		SetUniform(u, &value);
	}
}

void MaterialInternal::SetMatrix4(const std::string& name, const glm::mat4& value) {
	Uniform* u = GetUniform(name, VariantTypeMatrix4);
	if (u != nullptr && u->value.GetMatrix4() != value) {
		SetUniform(u, &value);
	}
}

int MaterialInternal::GetInt(const std::string& name) {
	Uniform* u = GetUniform(name, VariantTypeInt);
	if (u == nullptr) {
		return 0;
	}

	return u->value.GetInt();
}

float MaterialInternal::GetFloat(const std::string& name) {
	Uniform* u = GetUniform(name, VariantTypeFloat);
	if (u == nullptr) {
		return 0.f;
	}

	return u->value.GetFloat();
}

Texture MaterialInternal::GetTexture(const std::string& name) {
	Uniform* u = GetUniform(name, VariantTypeTexture);
	if (u == nullptr) {
		return Texture();
	}

	return u->value.GetTexture();
}

glm::mat4 MaterialInternal::GetMatrix4(const std::string& name) {
	Uniform* u = GetUniform(name, VariantTypeMatrix4);
	if (u == nullptr) {
		return glm::mat4(1);
	}

	return u->value.GetMatrix4();
}

glm::vec3 MaterialInternal::GetVector3(const std::string& name) {
	Uniform* u = GetUniform(name, VariantTypeVector3);
	if (u == nullptr) {
		return glm::vec3(0);
	}

	return u->value.GetVector3();
}

void MaterialInternal::Bind() {
	BindTextures();
	glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram_);
	glUseProgram(shader_->GetNativePointer());
}

void MaterialInternal::Unbind() {
	UnbindTextures();
	glUseProgram(oldProgram_);
	oldProgram_ = 0;
}

void MaterialInternal::Define(const std::string& name) {
}

void MaterialInternal::Undefine(const std::string& name) {
}

MaterialInternal::Uniform* MaterialInternal::GetUniform(const std::string& name, VariantType type) {
	Uniform* ans = nullptr;
	if (!uniforms_.get(name, ans)) {
		static int variablePrefixLength = strlen(VARIABLE_PREFIX);
		if (strncmp(name.c_str(), VARIABLE_PREFIX, variablePrefixLength) != 0) {
			Debug::LogWarning("uniform " + name + " does not exist.");
		}

		return false;
	}

	if (ans->value.GetType() != type) {
		Debug::LogError("uniform " + name + "does not defined as " + Variant::TypeString(type));
		return false;
	}

	return ans;
}

void MaterialInternal::UpdateVariables() {
	// http://www.lighthouse3d.com/tutorials/glsl-tutorial/uniform-blocks/
	AddAllUniforms();
}

void MaterialInternal::UpdateVertexAttributes() {
	GLuint program = shader_->GetNativePointer();
	glBindAttribLocation(program, VertexAttributeIndexPosition, Variables::position);
	glBindAttribLocation(program, VertexAttributeIndexTexCoord, Variables::texCoord);
	glBindAttribLocation(program, VertexAttributeIndexNormal, Variables::normal);
	glBindAttribLocation(program, VertexAttributeIndexTangent, Variables::tangent);
	glBindAttribLocation(program, VertexAttributeIndexBoneIndexes, Variables::boneIndexes);
	glBindAttribLocation(program, VertexAttributeIndexBoneWeights, Variables::boneWeights);

	glBindAttribLocation(program, VertexAttributeIndexUser0, Variables::user0);
	glBindAttribLocation(program, VertexAttributeIndexUser1, Variables::user1);
	glBindAttribLocation(program, VertexAttributeIndexUser2, Variables::user2);
}

void MaterialInternal::UpdateFragmentAttributes() {
	GLuint program = shader_->GetNativePointer();
	// TODO: bind fragment output.
	//glBindFragDataLocation(program, 0, Variables::fragColor);
	glBindFragDataLocation(program, 0, Variables::depth);
}

void MaterialInternal::BindTextures() {
	// TODO: traversal...
	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		Uniform* uniform = ite->second;
		if (uniform->value.GetType() == VariantTypeTexture && uniform->value.GetTexture()) {
			uniform->value.GetTexture()->Bind(GL_TEXTURE0 + uniform->value.GetTextureIndex());
		}
	}
}

void MaterialInternal::UnbindTextures() {
	// TODO: traversal...
	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		Uniform* uniform = ite->second;
		if (uniform->value.GetType() == VariantTypeTexture && uniform->value.GetTexture()) {
			uniform->value.GetTexture()->Unbind();
		}
	}
}

bool MaterialInternal::IsSampler(int type) {
	switch (type) {
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_SAMPLER_BUFFER:
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_INT_SAMPLER_BUFFER:
		case GL_INT_SAMPLER_2D_RECT:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
			return true;
	}

	return false;
}

void MaterialInternal::AddAllUniforms() {
	uniforms_.clear();

	GLenum type;
	GLuint location = 0;
	GLint size, count, maxLength, length;

	GLuint program = shader_->GetNativePointer();
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

	char* name = Memory::CreateArray<char>(maxLength);
	for (int i = 0; i < count; ++i) {
		glGetActiveUniform(program, i, maxLength, &length, &size, &type, name);

		location = glGetUniformLocation(program, name);

		// -1 indicates that is not an active uniform, although it may be present in a
		// uniform block.
		if (location == GL_INVALID_INDEX) {
			continue;
		}

		// TODO: uniform array.
		char* ptr = strrchr(name, '[');
		if (ptr != nullptr) {
			*ptr = 0;
		}

		AddUniform(name, type, location, size);
	}

	Memory::ReleaseArray(name);
}

void MaterialInternal::AddUniform(const char* name, GLenum type, GLuint location, GLint size) {
	Uniform* uniform = uniforms_[name];
	uniform->type = type;
	uniform->location = location;
	uniform->size = size;
	if (type == GL_INT) {
		uniform->value.SetInt(0);
	}
	else if (type == GL_FLOAT) {
		uniform->value.SetFloat(0);
	}
	else if (type == GL_FLOAT_MAT4) {
		uniform->value.SetMatrix4(glm::mat4(1));
	}
	else if (type == GL_BOOL) {
		uniform->value.SetBool(false);
	}
	else if (type == GL_FLOAT_VEC3) {
		uniform->value.SetVector3(glm::vec3(0));
	}
	else if (IsSampler(type)) {
		AssertX(textureUnitIndex_ < maxTextureUnits_, "too many textures.");
		uniform->value.SetTextureLocation(textureUnitIndex_++);
	}
	else {
		Debug::LogError(String::Format("undefined uniform type 0x%x.", type));
	}
}

GLuint MaterialInternal::GetUniformSize(GLint uniformType, GLint uniformSize,
	GLint uniformOffset, GLint uniformMatrixStride, GLint uniformArrayStride) {
	if (uniformArrayStride > 0) {
		return uniformSize * uniformArrayStride;
	}

	if (uniformMatrixStride > 0) {
		switch (uniformType) {
			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT2x3:
			case GL_FLOAT_MAT2x4:
			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT2x3:
			case GL_DOUBLE_MAT2x4:
				return 2 * uniformMatrixStride;

			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT3x2:
			case GL_FLOAT_MAT3x4:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT3x2:
			case GL_DOUBLE_MAT3x4:
				return 3 * uniformMatrixStride;

			case GL_FLOAT_MAT4:
			case GL_FLOAT_MAT4x2:
			case GL_FLOAT_MAT4x3:
			case GL_DOUBLE_MAT4:
			case GL_DOUBLE_MAT4x2:
			case GL_DOUBLE_MAT4x3:
				return 4 * uniformMatrixStride;

			default:
				Debug::LogError("invalid uniformType " + std::to_string(uniformType) + ".");
				return 0;
		}
	}

	return GetSizeOfType(uniformType);
}

GLuint MaterialInternal::GetSizeOfType(GLint type) {
	if (IsSampler(type)) {
		return sizeof(int);
	}

	switch (type) {
		case GL_FLOAT:
			return sizeof(float);

		case GL_FLOAT_VEC2:
			return sizeof(float) * 2;

		case GL_FLOAT_VEC3:
			return sizeof(float) * 3;

		case GL_FLOAT_VEC4:
			return sizeof(float) * 4;

			// Doubles
		case GL_DOUBLE:
			return sizeof(double);

		case GL_DOUBLE_VEC2:
			return sizeof(double) * 2;

		case GL_DOUBLE_VEC3:
			return sizeof(double) * 3;

		case GL_DOUBLE_VEC4:
			return sizeof(double) * 4;

			// ints and bools.
		case GL_INT:
		case GL_BOOL:
			return sizeof(int);

		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			return sizeof(int) * 2;

		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			return sizeof(int) * 3;

		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			return sizeof(int) * 4;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			return sizeof(unsigned int);

		case GL_UNSIGNED_INT_VEC2:
			return sizeof(unsigned int) * 2;

		case GL_UNSIGNED_INT_VEC3:
			return sizeof(unsigned int) * 3;

		case GL_UNSIGNED_INT_VEC4:
			return sizeof(unsigned int) * 4;

			// Float Matrices
		case GL_FLOAT_MAT2:
			return sizeof(float) * 4;

		case GL_FLOAT_MAT3:
			return sizeof(float) * 9;

		case GL_FLOAT_MAT4:
			return sizeof(float) * 16;

		case GL_FLOAT_MAT2x3:
			return sizeof(float) * 6;

		case GL_FLOAT_MAT2x4:
			return sizeof(float) * 8;

		case GL_FLOAT_MAT3x2:
			return sizeof(float) * 6;

		case GL_FLOAT_MAT3x4:
			return sizeof(float) * 12;

		case GL_FLOAT_MAT4x2:
			return sizeof(float) * 8;

		case GL_FLOAT_MAT4x3:
			return sizeof(float) * 12;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			return sizeof(double) * 4;

		case GL_DOUBLE_MAT3:
			return sizeof(double) * 9;

		case GL_DOUBLE_MAT4:
			return sizeof(double) * 16;

		case GL_DOUBLE_MAT2x3:
			return sizeof(double) * 6;

		case GL_DOUBLE_MAT2x4:
			return sizeof(double) * 8;

		case GL_DOUBLE_MAT3x2:
			return sizeof(double) * 6;

		case GL_DOUBLE_MAT3x4:
			return sizeof(double) * 12;

		case GL_DOUBLE_MAT4x2:
			return sizeof(double) * 8;

		case GL_DOUBLE_MAT4x3:
			return sizeof(double) * 12;

		default:
			Debug::LogError("invalid uniform type " + std::to_string(type) + ".");
			return 0;
	}

	return 0;
}

void MaterialInternal::SetUniform(Uniform* u, const void* value) {
	GLuint program = shader_->GetNativePointer();

	switch (u->type) {
		// Floats
		case GL_FLOAT:
			glProgramUniform1fv(program, u->location, u->size, (const GLfloat *)value);
			break;
		case GL_FLOAT_VEC2:
			glProgramUniform2fv(program, u->location, u->size, (const GLfloat *)value);
			break;
		case GL_FLOAT_VEC3:
			glProgramUniform3fv(program, u->location, u->size, (const GLfloat *)value);
			break;
		case GL_FLOAT_VEC4:
			glProgramUniform4fv(program, u->location, u->size, (const GLfloat *)value);
			break;

			// Doubles
		case GL_DOUBLE:
			glProgramUniform1dv(program, u->location, u->size, (const GLdouble *)value);
			break;
		case GL_DOUBLE_VEC2:
			glProgramUniform2dv(program, u->location, u->size, (const GLdouble *)value);
			break;
		case GL_DOUBLE_VEC3:
			glProgramUniform3dv(program, u->location, u->size, (const GLdouble *)value);
			break;
		case GL_DOUBLE_VEC4:
			glProgramUniform4dv(program, u->location, u->size, (const GLdouble *)value);
			break;

			// Samplers, Ints and Bools
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_SAMPLER_BUFFER:
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_INT_SAMPLER_BUFFER:
		case GL_INT_SAMPLER_2D_RECT:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
		case GL_BOOL:
		case GL_INT:
			glProgramUniform1iv(program, u->location, u->size, (const GLint *)value);
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			glProgramUniform2iv(program, u->location, u->size, (const GLint *)value);
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			glProgramUniform3iv(program, u->location, u->size, (const GLint *)value);
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			glProgramUniform4iv(program, u->location, u->size, (const GLint *)value);
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			glProgramUniform1uiv(program, u->location, u->size, (const GLuint *)value);
			break;
		case GL_UNSIGNED_INT_VEC2:
			glProgramUniform2uiv(program, u->location, u->size, (const GLuint *)value);
			break;
		case GL_UNSIGNED_INT_VEC3:
			glProgramUniform3uiv(program, u->location, u->size, (const GLuint *)value);
			break;
		case GL_UNSIGNED_INT_VEC4:
			glProgramUniform4uiv(program, u->location, u->size, (const GLuint *)value);
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			glProgramUniformMatrix2fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT3:
			glProgramUniformMatrix3fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT4:
			glProgramUniformMatrix4fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT2x3:
			glProgramUniformMatrix2x3fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT2x4:
			glProgramUniformMatrix2x4fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT3x2:
			glProgramUniformMatrix3x2fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT3x4:
			glProgramUniformMatrix3x4fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT4x2:
			glProgramUniformMatrix4x2fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;
		case GL_FLOAT_MAT4x3:
			glProgramUniformMatrix4x3fv(program, u->location, u->size, false, (const GLfloat *)value);
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			glProgramUniformMatrix2dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT3:
			glProgramUniformMatrix3dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT4:
			glProgramUniformMatrix4dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT2x3:
			glProgramUniformMatrix2x3dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT2x4:
			glProgramUniformMatrix2x4dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT3x2:
			glProgramUniformMatrix3x2dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT3x4:
			glProgramUniformMatrix3x4dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT4x2:
			glProgramUniformMatrix4x2dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
		case GL_DOUBLE_MAT4x3:
			glProgramUniformMatrix4x3dv(program, u->location, u->size, false, (const GLdouble *)value);
			break;
	}
}
