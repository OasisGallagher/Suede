#include "debug.h"
#include "math2.h"
#include "variables.h"
#include "renderstate.h"
#include "tools/string.h"
#include "materialinternal.h"

// empty storage for clearing uniform.
static float zeroBuffer[4 * 4];

MaterialInternal::MaterialInternal()
	: ObjectInternal(ObjectTypeMaterial)
	, textureUnitIndex_(0), maxTextureUnits_(0), oldProgram_(0) {
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits_);
	std::fill(states_, states_ + RenderStateCount, nullptr);
}

MaterialInternal::~MaterialInternal() {
	for (int i = 0; i < RenderStateCount; ++i) {
		MEMORY_RELEASE(states_[i]);
	}
}

Object MaterialInternal::Clone() {
	Material material = NewMaterial();
	MaterialInternal* ptr = dynamic_cast<MaterialInternal*>(material.get());
	ptr->shader_ = shader_;
	ptr->oldProgram_ = 0;
	ptr->maxTextureUnits_ = maxTextureUnits_;
	ptr->textureUnitIndex_ = textureUnitIndex_;

	ptr->textureUniforms_.resize(textureUniforms_.size());

	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		Uniform* uniform = ptr->uniforms_[ite->first];
		memcpy(uniform, ite->second, sizeof(Uniform));
		std::vector<Uniform*>::iterator pos = std::find(textureUniforms_.begin(), textureUniforms_.end(), ite->second);
		if (pos != textureUniforms_.end()) {
			ptr->textureUniforms_[pos - textureUniforms_.begin()] = uniform;
		}
	}

	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			ptr->states_[i] = states_[i]->Clone();
		}
	}

	return material;
}

void MaterialInternal::SetShader(Shader value) {
	shader_ = value;
	
	UnbindUniforms();

	UpdateVertexAttributes();
	UpdateFragmentAttributes();

	shader_->Link();

	UpdateVariables();
}

void MaterialInternal::BindRenderStates() {
	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Bind();
		}
	}
}

void MaterialInternal::UnbindRenderStates() {
	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Unbind();
		}
	}
}

void MaterialInternal::SetInt(const std::string& name, int value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Uniform* u = GetUniform(name, VariantTypeInt);
	if (u != nullptr && u->value.GetInt() != value) {
		u->value.SetInt(value);
		glProgramUniform1i(shader_->GetNativePointer(), u->location, value);
	}
}

void MaterialInternal::SetFloat(const std::string& name, float value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Uniform* u = GetUniform(name, VariantTypeFloat);
	if (u != nullptr && !Math::Approximately(u->value.GetFloat(), value)) {
		u->value.SetFloat(value);
		glProgramUniform1f(shader_->GetNativePointer(), u->location, value);
	}
}

void MaterialInternal::SetTexture(const std::string& name, Texture value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Uniform* u = GetUniform(name, VariantTypeTexture);

	if (u != nullptr && u->value.GetTexture() != value) {
		u->value.SetTexture(value);
		glProgramUniform1i(shader_->GetNativePointer(), u->location, u->value.GetTextureIndex());
	}
}

void MaterialInternal::SetVector3(const std::string& name, const glm::vec3& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Uniform* u = GetUniform(name, VariantTypeVector3);
	if (u != nullptr && u->value.GetVector3() != value) {
		u->value.SetVector3(value);
	}
}

void MaterialInternal::SetVector4(const std::string& name, const glm::vec4& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Uniform* u = GetUniform(name, VariantTypeVector4);
	if (u != nullptr && u->value.GetVector4() != value) {
		u->value.SetVector4(value);
	}
}

void MaterialInternal::SetMatrix4(const std::string& name, const glm::mat4& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Uniform* u = GetUniform(name, VariantTypeMatrix4);
	if (u != nullptr && u->value.GetMatrix4() != value) {
		u->value.SetMatrix4(value);
	}
}

int MaterialInternal::GetInt(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	Uniform* u = GetUniform(name, VariantTypeInt);
	if (u == nullptr) {
		return 0;
	}

	return u->value.GetInt();
}

float MaterialInternal::GetFloat(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	Uniform* u = GetUniform(name, VariantTypeFloat);
	if (u == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return 0.f;
	}

	return u->value.GetFloat();
}

Texture MaterialInternal::GetTexture(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return nullptr;
	}

	Uniform* u = GetUniform(name, VariantTypeTexture);
	if (u == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return Texture();
	}

	return u->value.GetTexture();
}

glm::mat4 MaterialInternal::GetMatrix4(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return glm::mat4(1);
	}

	Uniform* u = GetUniform(name, VariantTypeMatrix4);
	if (u == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return glm::mat4(1);
	}

	return u->value.GetMatrix4();
}

glm::vec3 MaterialInternal::GetVector3(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return glm::vec3(0);
	}

	Uniform* u = GetUniform(name, VariantTypeVector3);
	if (u == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return glm::vec3(0);
	}

	return u->value.GetVector3();
}

glm::vec4 MaterialInternal::GetVector4(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return glm::vec4(0, 0, 0, 1);
	}

	Uniform* u = GetUniform(name, VariantTypeVector4);
	if (u == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return glm::vec4(0, 0, 0, 1);
	}

	return u->value.GetVector4();
}

void MaterialInternal::Bind() {
	BindRenderStates();
	BindUniforms();

	glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram_);
	glUseProgram(shader_->GetNativePointer());
}

void MaterialInternal::Unbind() {
	UnbindUniforms();
	UnbindRenderStates();

	glUseProgram(oldProgram_);
	oldProgram_ = 0;
}

void MaterialInternal::Define(const std::string& name) {
}

void MaterialInternal::Undefine(const std::string& name) {
}

void MaterialInternal::SetRenderState(RenderStateType type, int parameter0, int parameter1) {
	RenderState* state = nullptr;
	switch (type) {
		case Cull:
			state = MEMORY_CREATE(CullState);
			break;
		case DepthTest:
			state = MEMORY_CREATE(DepthTestState);
			break;
		case Blend:
			state = MEMORY_CREATE(BlendState);
			break;
		case DepthWrite:
			state = MEMORY_CREATE(DepthWriteState);
			break;
		case RasterizerDiscard:
			state = MEMORY_CREATE(RasterizerDiscardState);
			break;
		default:
			Debug::LogError("invalid render state %d.", type);
			break;
	}

	state->Initialize(parameter0, parameter1);
	MEMORY_RELEASE(states_[type]);
	states_[type] = state;
}

MaterialInternal::Uniform* MaterialInternal::GetUniform(const std::string& name, VariantType type) {
	Uniform* ans = nullptr;
	if (!uniforms_.get(name, ans)) {
		static int variablePrefixLength = strlen(VARIABLE_PREFIX);
		if (strncmp(name.c_str(), VARIABLE_PREFIX, variablePrefixLength) != 0) {
			Debug::LogWarning("property %s does not exist.", name.c_str());
		}

		return false;
	}

	if (ans->value.GetType() != type) {
		Debug::LogError("property %s does not defined as %s.", name.c_str(), Variant::TypeString(type).c_str());
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
	glBindAttribLocation(program, VertexAttribPosition, Variables::position);
	glBindAttribLocation(program, VertexAttribTexCoord, Variables::texCoord);
	glBindAttribLocation(program, VertexAttribNormal, Variables::normal);
	glBindAttribLocation(program, VertexAttribTangent, Variables::tangent);
	glBindAttribLocation(program, VertexAttribBoneIndexes, Variables::boneIndexes);
	glBindAttribLocation(program, VertexAttribBoneWeights, Variables::boneWeights);

	glBindAttribLocation(program, VertexAttribInstanceColor, Variables::instanceColor);
	glBindAttribLocation(program, VertexAttribInstanceGeometry, Variables::instanceGeometry);
}

void MaterialInternal::UpdateFragmentAttributes() {
	GLuint program = shader_->GetNativePointer();
	glBindFragDataLocation(program, 0, Variables::depth);
	glBindFragDataLocation(program, 0, Variables::fragColor);
}

void MaterialInternal::BindUniforms() {
	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		SetUniform(ite->second, ite->second->value.GetData());
	}

	for (int i = 0; i < textureUniforms_.size(); ++i) {
		Uniform* uniform = textureUniforms_[i];
		if (uniform->value.GetTexture()) {
			uniform->value.GetTexture()->Bind(uniform->value.GetTextureIndex());
		}
	}
}

void MaterialInternal::UnbindUniforms() {
	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		SetUniform(ite->second, &zeroBuffer);
	}

	for (int i = 0; i < textureUniforms_.size(); ++i) {
		Uniform* uniform = textureUniforms_[i];
		if (uniform->value.GetTexture()) {
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
	textureUniforms_.clear();

	GLenum type;
	GLuint location = 0;
	GLint size, count, maxLength, length;

	GLuint program = shader_->GetNativePointer();
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

	char* name = MEMORY_CREATE_ARRAY(char, maxLength);
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

	MEMORY_RELEASE_ARRAY(name);
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
		uniform->value.SetMatrix4(glm::mat4(0));
	}
	else if (type == GL_BOOL) {
		uniform->value.SetBool(false);
	}
	else if (type == GL_FLOAT_VEC3) {
		uniform->value.SetVector3(glm::vec3(0));
	}
	else if (type == GL_FLOAT_VEC4) {
		uniform->value.SetVector4(glm::vec4(0, 0, 0, 1));
	}
	else if (IsSampler(type)) {
		if (textureUnitIndex_ >= maxTextureUnits_) {
			Debug::LogError("too many textures.");
		}
		else {
			uniform->value.SetTextureIndex(textureUnitIndex_++);
			textureUniforms_.push_back(uniform);
		}
	}
	else {
		Debug::LogError("undefined uniform type 0x%x.", type);
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
				Debug::LogError("invalid uniformType %d.", uniformType);
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
			return sizeof(uint);

		case GL_UNSIGNED_INT_VEC2:
			return sizeof(uint) * 2;

		case GL_UNSIGNED_INT_VEC3:
			return sizeof(uint) * 3;

		case GL_UNSIGNED_INT_VEC4:
			return sizeof(uint) * 4;

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
			Debug::LogError("invalid uniform type %d.", type);
			return 0;
	}

	return 0;
}

void MaterialInternal::SetUniform(Uniform* u, const void* data) {
	GLuint program = shader_->GetNativePointer();

	if (IsSampler(u->type)) {
		glProgramUniform1iv(program, u->location, u->size, (const GLint *)data);
		return;
	}

	switch (u->type) {
		// Floats
		case GL_FLOAT:
			glProgramUniform1fv(program, u->location, u->size, (const GLfloat *)data);
			break;
		case GL_FLOAT_VEC2:
			glProgramUniform2fv(program, u->location, u->size, (const GLfloat *)data);
			break;
		case GL_FLOAT_VEC3:
			glProgramUniform3fv(program, u->location, u->size, (const GLfloat *)data);
			break;
		case GL_FLOAT_VEC4:
			glProgramUniform4fv(program, u->location, u->size, (const GLfloat *)data);
			break;

			// Doubles
		case GL_DOUBLE:
			glProgramUniform1dv(program, u->location, u->size, (const GLdouble *)data);
			break;
		case GL_DOUBLE_VEC2:
			glProgramUniform2dv(program, u->location, u->size, (const GLdouble *)data);
			break;
		case GL_DOUBLE_VEC3:
			glProgramUniform3dv(program, u->location, u->size, (const GLdouble *)data);
			break;
		case GL_DOUBLE_VEC4:
			glProgramUniform4dv(program, u->location, u->size, (const GLdouble *)data);
			break;

			// Ints and Bools
		case GL_BOOL:
		case GL_INT:
			glProgramUniform1iv(program, u->location, u->size, (const GLint *)data);
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			glProgramUniform2iv(program, u->location, u->size, (const GLint *)data);
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			glProgramUniform3iv(program, u->location, u->size, (const GLint *)data);
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			glProgramUniform4iv(program, u->location, u->size, (const GLint *)data);
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			glProgramUniform1uiv(program, u->location, u->size, (const GLuint *)data);
			break;
		case GL_UNSIGNED_INT_VEC2:
			glProgramUniform2uiv(program, u->location, u->size, (const GLuint *)data);
			break;
		case GL_UNSIGNED_INT_VEC3:
			glProgramUniform3uiv(program, u->location, u->size, (const GLuint *)data);
			break;
		case GL_UNSIGNED_INT_VEC4:
			glProgramUniform4uiv(program, u->location, u->size, (const GLuint *)data);
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			glProgramUniformMatrix2fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT3:
			glProgramUniformMatrix3fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT4:
			glProgramUniformMatrix4fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT2x3:
			glProgramUniformMatrix2x3fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT2x4:
			glProgramUniformMatrix2x4fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT3x2:
			glProgramUniformMatrix3x2fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT3x4:
			glProgramUniformMatrix3x4fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT4x2:
			glProgramUniformMatrix4x2fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;
		case GL_FLOAT_MAT4x3:
			glProgramUniformMatrix4x3fv(program, u->location, u->size, false, (const GLfloat *)data);
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			glProgramUniformMatrix2dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT3:
			glProgramUniformMatrix3dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT4:
			glProgramUniformMatrix4dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT2x3:
			glProgramUniformMatrix2x3dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT2x4:
			glProgramUniformMatrix2x4dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT3x2:
			glProgramUniformMatrix3x2dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT3x4:
			glProgramUniformMatrix3x4dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT4x2:
			glProgramUniformMatrix4x2dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
		case GL_DOUBLE_MAT4x3:
			glProgramUniformMatrix4x3dv(program, u->location, u->size, false, (const GLdouble *)data);
			break;
	}
}
