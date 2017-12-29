#include "variables.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "meshinternal.h"
#include "shaderinternal.h"
#include "internal/base/glsldefines.h"

Pass::Pass() : program_(0) {
	std::fill(states_, states_ + RenderStateCount, nullptr);

	program_ = GL::CreateProgram();
	GL::GetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits_);

	std::fill(shaderObjs_, shaderObjs_ + ShaderStageCount, 0);
}

Pass::~Pass() {
	for (int i = 0; i < RenderStateCount; ++i) {
		MEMORY_RELEASE(states_[i]);
	}

	GL::DeleteProgram(program_);
	ClearIntermediateShaders();
}

bool Pass::Initialize(const Semantics::Pass& pass, const std::string& path) {
	InitializeRenderStates(pass.renderStates);
	
	std::string sources[ShaderStageCount];
	GLSLParser parser;
	// TODO: SetPath.
	parser.Parse(sources, pass.source, "");

	for (int i = 0; i < ShaderStageCount; ++i) {
		if (!sources[i].empty() && !LoadSource((ShaderStage)i, sources[i].c_str())) {
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
}

void Pass::InitializeRenderStates(std::vector<Semantics::RenderState> states) {

}

bool Pass::GetErrorMessage(GLuint shaderObj, std::string& answer) {
	if (shaderObj == 0) {
		answer = "invalid shader id";
		return false;
	}

	GLint length = 0, writen = 0;
	GL::GetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &length);
	if (length > 1) {
		answer.resize(length);
		GL::GetShaderInfoLog(shaderObj, length, &writen, &answer[0]);
		return true;
	}

	return false;
}

bool Pass::Link() {
	GL::LinkProgram(program_);

	GLint status = GL_FALSE;
	GL::GetProgramiv(program_, GL_LINK_STATUS, &status);

	if (status != GL_TRUE) {
		Debug::LogError("failed to link shader %s.", path_.c_str());
		return false;
	}

	GL::ValidateProgram(program_);
	GL::GetProgramiv(program_, GL_VALIDATE_STATUS, &status);
	if (status != GL_TRUE) {
		Debug::LogWarning("failed to validate shader %s.", path_.c_str());
		//return false;
	}

	return true;
}

void Pass::ClearIntermediateShaders() {
	for (int i = 0; i < ShaderStageCount; ++i) {
		if (shaderObjs_[i] != 0) {
			GL::DeleteShader(shaderObjs_[i]);
			shaderObjs_[i] = 0;
		}
	}
}

bool Pass::LoadSource(ShaderStage stage, const char* source) {
	GLuint shaderObj = GL::CreateShader(GetShaderDescription(stage).glShaderStage);

	GL::ShaderSource(shaderObj, 1, &source, nullptr);
	GL::CompileShader(shaderObj);

	GL::AttachShader(program_, shaderObj);

	std::string message;
	if (!GetErrorMessage(shaderObj, message)) {
		if (shaderObjs_[stage] != 0) {
			GL::DeleteShader(shaderObjs_[stage]);
		}
		shaderObjs_[stage] = shaderObj;
		return true;
	}

	Debug::LogError("%s %s.", GetShaderDescription(stage).name, message.c_str());
	return false;
}

void Pass::UpdateVertexAttributes() {
	GL::BindAttribLocation(program_, VertexAttribPosition, Variables::position);
	GL::BindAttribLocation(program_, VertexAttribTexCoord, Variables::texCoord);
	GL::BindAttribLocation(program_, VertexAttribNormal, Variables::normal);
	GL::BindAttribLocation(program_, VertexAttribTangent, Variables::tangent);
	GL::BindAttribLocation(program_, VertexAttribBoneIndexes, Variables::boneIndexes);
	GL::BindAttribLocation(program_, VertexAttribBoneWeights, Variables::boneWeights);

	GL::BindAttribLocation(program_, VertexAttribInstanceColor, Variables::instanceColor);
	GL::BindAttribLocation(program_, VertexAttribInstanceGeometry, Variables::instanceGeometry);
}

void Pass::UpdateFragmentAttributes() {
}

void Pass::AddAllUniforms() {
	uniforms_.clear();
	textureUnitCount_ = 0;

	GLenum type;
	GLuint location = 0;
	GLint size, count, maxLength, length;

	GL::GetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);
	GL::GetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

	char* name = MEMORY_CREATE_ARRAY(char, maxLength);
	for (int i = 0; i < count; ++i) {
		GL::GetActiveUniform(program_, i, maxLength, &length, &size, &type, name);

		location = GL::GetUniformLocation(program_, name);

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

void Pass::AddUniform(const char* name, GLenum type, GLuint location, GLint size) {
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

void Pass::SetUniform(Uniform* uniform, const void* data) {
	switch (uniform->type) {
		case ShaderPropertyTypeInt:
		case ShaderPropertyTypeBool:
			GL::ProgramUniform1iv(program_, uniform->location, uniform->size, (const GLint *)data);
			break;
		case ShaderPropertyTypeFloat:
			GL::ProgramUniform1fv(program_, uniform->location, uniform->size, (const GLfloat *)data);
			break;
		case ShaderPropertyTypeMatrix4:
		case ShaderPropertyTypeMatrix4Array:
			GL::ProgramUniformMatrix4fv(program_, uniform->location, uniform->size, false, (const GLfloat *)data);
			break;
		case ShaderPropertyTypeVector3:
			GL::ProgramUniform3fv(program_, uniform->location, uniform->size, (const GLfloat *)data);
			break;
		case ShaderPropertyTypeVector4:
			GL::ProgramUniform4fv(program_, uniform->location, uniform->size, (const GLfloat *)data);
			break;
		default:
			Debug::LogError("unable to set uniform (type 0x%x).", uniform->type);
			break;
	}
}

bool Pass::IsSampler(int type) {
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


SubShader::SubShader() : passes_(nullptr), passCount_(0), tags_(nullptr), tagCount_(0) {
}

SubShader::~SubShader() {
	MEMORY_RELEASE_ARRAY(tags_);
	MEMORY_RELEASE_ARRAY(passes_);
}

bool SubShader::Initialize(const Semantics::SubShader& config, const std::string& path) {
	tags_ = MEMORY_CREATE_ARRAY(Tag, config.tags.size());

	InitializeTags(config.tags);

	passCount_ = config.tags.size();
	passes_ = MEMORY_CREATE_ARRAY(Pass, config.passes.size());

	for (uint i = 0; i < passCount_; ++i) {
		passes_[i].Initialize(config.passes[i], path);
	}

	return true;
}

void SubShader::InitializeTags(const std::vector<Semantics::Tag>& tags) {
	for (uint i = 0; i < tagCount_; ++i) {
		if (tags[i].key == "Queue") {
			tags_[i].key = TagKeyQueue;
		}
		else {
			Debug::LogError("invalid key %s", tags[i].key.c_str());
		}
	}
}

ShaderInternal::ShaderInternal() : ObjectInternal(ObjectTypeShader)
	, subShaderCount_(0), subShaders_(nullptr) {
}

ShaderInternal::~ShaderInternal() {
	MEMORY_RELEASE(subShaders_);
}

bool ShaderInternal::Load(const std::string& path) {
	ShaderParser parser;
	std::string sources[ShaderStageCount];

	Semantics semantics;
	if (!parser.Parse(semantics, "resources/demo.js"/*path + GLSL_POSTFIX*/, "")) {
		return false;
	}

	ParseProperties(semantics.properties);
	ParseSubShaders(semantics.subShaders, path);
	return true;
}

void ShaderInternal::ParseProperties(std::vector<Semantics::Property>& properties) {
	properties_ = properties;
}

void ShaderInternal::ParseSubShaders(std::vector<Semantics::SubShader>& subShaders, const std::string& path) {
	subShaderCount_ = subShaders.size();
	subShaders_ = MEMORY_CREATE_ARRAY(SubShader, subShaders.size());
	for (uint i = 0; i < subShaderCount_; ++i) {
		subShaders_[i].Initialize(subShaders[i], path);
	}
}

void ShaderInternal::Bind(uint pass) {
// 	currentPass_ = pass;
// 	passes_[pass]->Bind();
}

void ShaderInternal::Unbind() {
//	passes_[currentPass_]->Unbind();
}

bool ShaderInternal::SetProperty(const std::string& name, const void* data) {
// 	Uniform* uniform = nullptr;
// 	if (!uniforms_.get(name, uniform)) {
// 		return false;
// 	}
// 
// 	SetUniform(uniform, data);
	return true;
}

void ShaderInternal::GetProperties(std::vector<ShaderProperty>& properties) {
// 	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
// 		ShaderProperty property{ ite->first, ite->second->type };
// 		properties.push_back(property);
// 	}
}
