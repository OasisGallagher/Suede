#include "debug.h"
#include "variables.h"
#include "tools/string.h"
#include "meshinternal.h"
#include "shaderinternal.h"
#include "internal/base/glsldefines.h"
#include "internal/file/shadercompiler.h"

ShaderInternal::ShaderInternal() : ObjectInternal(ObjectTypeShader), textureUnitCount_(0){
	program_ = glCreateProgram();
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits_);

	std::fill(shaderObjs_, shaderObjs_ + ShaderTypeCount, 0);
}

ShaderInternal::~ShaderInternal() {
	glDeleteProgram(program_);
	ClearIntermediateShaders();
}

bool ShaderInternal::Load(const std::string& path) {
	ShaderCompiler compiler;
	std::string sources[ShaderTypeCount];
	if (!compiler.Compile(path + GLSL_POSTFIX, "", sources)) {
		return false;
	}

	for (int i = 0; i < ShaderTypeCount; ++i) {
		if (!sources[i].empty() && !LoadSource((ShaderType)i, sources[i].c_str())) {
			return false;
		}
	}

	path_ = path;
	ClearIntermediateShaders();
	
	UpdateVertexAttributes();
	UpdateFragmentAttributes();

	if (!Link()) {
		return false;
	}

	AddAllUniforms();
	return true;
}

bool ShaderInternal::SetProperty(const std::string& name, const void* data) {
	Uniform* uniform = nullptr;
	if (!uniforms_.get(name, uniform)) {
		return false;
	}

	SetUniform(uniform, data);
	return true;
}

void ShaderInternal::GetProperties(std::vector<ShaderProperty>& properties) {
	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		ShaderProperty property{ ite->first, ite->second->type };
		properties.push_back(property);
	}
}

bool ShaderInternal::GetErrorMessage(GLuint shaderObj, std::string& answer) {
	if (shaderObj == 0) {
		answer = "invalid shader id";
		return false;
	}

	GLint length = 0, writen = 0;
	glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &length);
	if (length > 1) {
		answer.resize(length);
		glGetShaderInfoLog(shaderObj, length, &writen, &answer[0]);
		return true;
	}

	return false;
}

bool ShaderInternal::Link() {
	glLinkProgram(program_);
	
	GLint status = GL_FALSE;
	glGetProgramiv(program_, GL_LINK_STATUS, &status);

	if (status != GL_TRUE) {
		Debug::LogError("failed to link shader %s.", path_.c_str());
		return false;
	}

	glValidateProgram(program_);
	glGetProgramiv(program_, GL_VALIDATE_STATUS, &status);
	if (status != GL_TRUE) {
		Debug::LogWarning("failed to validate shader %s.", path_.c_str());
		//return false;
	}

	return true;
}

void ShaderInternal::ClearIntermediateShaders() {
	for (int i = 0; i < ShaderTypeCount; ++i) {
		if (shaderObjs_[i] != 0) {
			glDeleteShader(shaderObjs_[i]);
			shaderObjs_[i] = 0;
		}
	}
}

bool ShaderInternal::LoadSource(ShaderType shaderType, const char* source) {
	GLuint shaderObj = glCreateShader(GetShaderDescription(shaderType).glShaderType);

	glShaderSource(shaderObj, 1, &source, nullptr);
	glCompileShader(shaderObj);

	glAttachShader(program_, shaderObj);
	
	std::string message;
	if (!GetErrorMessage(shaderObj, message)) {
		if (shaderObjs_[shaderType] != 0) {
			glDeleteShader(shaderObjs_[shaderType]);
		}
		shaderObjs_[shaderType] = shaderObj;
		return true;
	}

	Debug::LogError("%s %s.", GetShaderDescription(shaderType).name, message.c_str());
	return false;
}

void ShaderInternal::UpdateVertexAttributes() {
	glBindAttribLocation(program_, VertexAttribPosition, Variables::position);
	glBindAttribLocation(program_, VertexAttribTexCoord, Variables::texCoord);
	glBindAttribLocation(program_, VertexAttribNormal, Variables::normal);
	glBindAttribLocation(program_, VertexAttribTangent, Variables::tangent);
	glBindAttribLocation(program_, VertexAttribBoneIndexes, Variables::boneIndexes);
	glBindAttribLocation(program_, VertexAttribBoneWeights, Variables::boneWeights);

	glBindAttribLocation(program_, VertexAttribInstanceColor, Variables::instanceColor);
	glBindAttribLocation(program_, VertexAttribInstanceGeometry, Variables::instanceGeometry);
}

void ShaderInternal::UpdateFragmentAttributes() {
	glBindFragDataLocation(program_, 0, Variables::depth);
	glBindFragDataLocation(program_, 0, Variables::fragColor);
}

void ShaderInternal::AddAllUniforms() {
	uniforms_.clear();
	textureUnitCount_ = 0;

	GLenum type;
	GLuint location = 0;
	GLint size, count, maxLength, length;

	glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);
	glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

	char* name = MEMORY_CREATE_ARRAY(char, maxLength);
	for (int i = 0; i < count; ++i) {
		glGetActiveUniform(program_, i, maxLength, &length, &size, &type, name);

		location = glGetUniformLocation(program_, name);

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

void ShaderInternal::AddUniform(const char* name, GLenum type, GLuint location, GLint size) {
	Uniform* uniform = uniforms_[name];
	uniform->size = size;
	uniform->location = location;

	if (type == GL_INT) {
		uniform->type = ShaderPropertyTypeInt;
	}
	else if (type == GL_FLOAT) {
		uniform->type = ShaderPropertyTypeFloat;
	}
	else if (type == GL_FLOAT_MAT4) {
		uniform->type = (size == 1) ? ShaderPropertyTypeMatrix4 : ShaderPropertyTypeMatrix4Array;
	}
	else if (type == GL_BOOL) {
		uniform->type = ShaderPropertyTypeBool;
	}
	else if (type == GL_FLOAT_VEC3) {
		uniform->type = ShaderPropertyTypeVector3;
	}
	else if (type == GL_FLOAT_VEC4) {
		uniform->type = ShaderPropertyTypeVector4;
	}
	else if (IsSampler(type)) {
		if (textureUnitCount_ >= maxTextureUnits_) {
			Debug::LogError("too many textures.");
		}
		else {
			++textureUnitCount_;
			uniform->type = ShaderPropertyTypeTexture;
		}
	}
	else {
		Debug::LogError("undefined uniform type 0x%x.", type);
	}
}

void ShaderInternal::SetUniform(Uniform* uniform, const void* data) {
	switch (uniform->type) {
		case ShaderPropertyTypeInt:
		case ShaderPropertyTypeBool:
			glProgramUniform1iv(program_, uniform->location, uniform->size, (const GLint *)data);
			break;
		case ShaderPropertyTypeFloat:
			glProgramUniform1fv(program_, uniform->location, uniform->size, (const GLfloat *)data);
			break;
		case ShaderPropertyTypeMatrix4:
		case ShaderPropertyTypeMatrix4Array:
			glProgramUniformMatrix4fv(program_, uniform->location, uniform->size, false, (const GLfloat *)data);
			break;
		case ShaderPropertyTypeVector3:
			glProgramUniform3fv(program_, uniform->location, uniform->size, (const GLfloat *)data);
			break;
		case ShaderPropertyTypeVector4:
			glProgramUniform4fv(program_, uniform->location, uniform->size, (const GLfloat *)data);
			break;
		default:
			Debug::LogError("unable to set uniform (type 0x%x).", uniform->type);
			break;
	}
}

bool ShaderInternal::IsSampler(int type) {
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
