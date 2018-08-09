#include "glef.h"
#include "variables.h"
#include "tools/math2.h"
#include "vertexattrib.h"
#include "tools/string.h"
#include "os/filesystem.h"
#include "../api/glutils.h"
#include "shaderinternal.h"

#include "internal/base/renderdefines.h"

#include "internal/rendering/uniformbuffermanager.h"

std::pair<std::string, float> _variables[] = {
	std::make_pair("Background", (float)RenderQueueBackground),
	std::make_pair("Geometry", (float)RenderQueueGeometry),
	std::make_pair("Transparent", (float)RenderQueueTransparent),
	std::make_pair("Overlay", (float)RenderQueueOverlay),
};

static std::map<std::string, float> renderQueueVariables(_variables, _variables + CountOf(_variables));

#define BIND(old, new)	if (old == new) { old = -1; } else
#define UNBIND(old)		if (old == -1) { } else

Pass::Pass() : program_(0), oldProgram_(0) {
	program_ = GL::CreateProgram();

	std::fill(states_, states_ + RenderStateCount, nullptr);
	std::fill(shaderObjs_, shaderObjs_ + ShaderStageCount, 0);
}

Pass::~Pass() {
	for (int i = 0; i < RenderStateCount; ++i) {
		MEMORY_DELETE(states_[i]);
	}

	GL::DeleteProgram(program_);
	ClearIntermediateShaders();
}

bool Pass::Initialize(std::vector<Property*>& properties, const Semantics::Pass& pass, const std::string& path) {
	name_ = pass.name;

	InitializeRenderStates(pass.renderStates);
	
	std::string sources[ShaderStageCount];

	GLSLParser parser;
	if (!parser.Parse(sources, path, pass.source, "")) {
		return false;
	}

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
	AddAllUniformProperties(properties);

	return true;
}

bool Pass::SetProperty(const std::string& name, const void* data) {
	Uniform* uniform = nullptr;
	if (!uniforms_.get(name, uniform)) {
		return false;
	}

	SetUniform(uniform, data);
	return true;
}

void Pass::Bind() {
	BindRenderStates();
	GL::GetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&oldProgram_);
	BIND(oldProgram_, program_) {
		GL::UseProgram(program_);
	}
}

void Pass::Unbind() {
	UnbindRenderStates();
	UNBIND(oldProgram_) {
		GL::UseProgram(oldProgram_);
	}
}

void Pass::InitializeRenderStates(const std::vector<Semantics::RenderState>& states) {
	for (uint i = 0; i < states.size(); ++i) {
		RenderState* state = CreateRenderState(states[i]);
		MEMORY_DELETE(states_[state->GetType()]);
		states_[state->GetType()] = state;
	}
}

void Pass::BindRenderStates() {
	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Bind();
		}
	}
}

void Pass::UnbindRenderStates() {
	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Unbind();
		}
	}
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
		// harmless.
		//Debug::LogWarning("failed to validate shader %s.", path_.c_str());
		//return false;
	}

	return true;
}

RenderState* Pass::CreateRenderState(const Semantics::RenderState& state) {
	int parameters[Semantics::RenderState::ParameterCount];
	if (!ParseRenderStateParameters(parameters, state.parameters)) {
		return nullptr;
	}

	RenderState* answer = AllocateRenderState(state);
	if (answer != nullptr) {
		answer->Initialize(parameters[0], parameters[1], parameters[2]);
	}

	return answer;
}

RenderState* Pass::AllocateRenderState(const Semantics::RenderState &state) {
#define CASE(name)	if (state.type == #name) return MEMORY_NEW(name ## State)
	CASE(Cull);
	CASE(ZTest);
	CASE(Offset);
	CASE(Blend);
	CASE(ZWrite);
	CASE(StencilOp);
	CASE(StencilTest);
	CASE(StencilWrite);
	CASE(RasterizerDiscard);
#undef CASE

	Debug::LogError("invalid render state %s.", state.type.c_str());
	return nullptr;
}

bool Pass::ParseRenderStateParameters(int* answer, const std::string* parameters) {
	for (uint i = 0; i < Semantics::RenderState::ParameterCount; ++i, ++answer) {
		if (!RenderStateParameterToInteger(parameters[i],*answer)) {
			return false;
		}
	}

	return true;
}

bool Pass::RenderStateParameterToInteger(const std::string& parameter, int& answer) {
	if (parameter.empty()) {
		answer = None;
		return true;
	}

#define CASE(value)	if (parameter == #value) { answer = value; return true; } else (void)0
	CASE(None);
	CASE(Front); 
	CASE(Back);
	CASE(FrontAndBack); 
	CASE(On); 
	CASE(Off);
	CASE(Never);
	CASE(Less);
	CASE(LEqual);
	CASE(Equal); 
	CASE(Greater);
	CASE(NotEqual); 
	CASE(GEqual); 
	CASE(Always);
	CASE(Zero); 
	CASE(One);
	CASE(SrcColor); 
	CASE(OneMinusSrcColor);
	CASE(SrcAlpha);
	CASE(OneMinusSrcAlpha);
	CASE(DestAlpha);
	CASE(OneMinusDestAlpha);
	CASE(Keep); 
	CASE(Replace);
	CASE(Incr);
	CASE(IncrWrap); 
	CASE(Decr);
	CASE(DecrWrap);
	CASE(Invert);
#undef CASE

	int integer = -1;
	if (!String::ToInteger(parameter, &integer)) {
		Debug::LogError("invalid render state parameter %s.", parameter.c_str());
		return false;
	}

	answer = integer;
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
	GL::BindAttribLocation(program_, VertexAttribPosition, Variables::Pos);
	GL::BindAttribLocation(program_, VertexAttribTexCoord, Variables::TexCoord);
	GL::BindAttribLocation(program_, VertexAttribNormal, Variables::Normal);
	GL::BindAttribLocation(program_, VertexAttribTangent, Variables::Tangent);
	GL::BindAttribLocation(program_, VertexAttribBoneIndexes, Variables::BoneIndexes);
	GL::BindAttribLocation(program_, VertexAttribBoneWeights, Variables::BoneWeights);

	GL::BindAttribLocation(program_, VertexAttribInstanceColor, Variables::InstanceColor);
	GL::BindAttribLocation(program_, VertexAttribInstanceGeometry, Variables::InstanceGeometry);

	// https://stackoverflow.com/questions/28818997/how-to-use-glvertexattrib
	// TODO: layout(location) must be set explicitly for glVertexAttrib* usage?
	// int location = glGetAttribLocation(program_, Variables::MatrixBuffer);
	// GL::BindAttribLocation(program_, VertexAttribMatrixOffset, Variables::MatrixTextureBuffer);
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

	char* name = MEMORY_NEW_ARRAY(char, maxLength);
	for (int i = 0; i < count; ++i) {
		GL::GetActiveUniform(program_, i, maxLength, &length, &size, &type, name);

		location = GL::GetUniformLocation(program_, name);

		// -1 indicates that is not an active uniform, although it may be present in a
		// uniform block.
		if (location == GL_INVALID_INDEX) {
			continue;
		}

		char* ptr = strrchr(name, '[');
		if (ptr != nullptr) { *ptr = 0; }

		AddUniform(name, type, location, size);
	}

	MEMORY_DELETE_ARRAY(name);
}

void Pass::AddAllUniformProperties(std::vector<Property*>& properties) {
	for (UniformContainer::iterator ite = uniforms_.begin(); ite != uniforms_.end(); ++ite) {
		AddUniformProperty(properties, ite->first, ite->second->type);
	}
}

void Pass::AddUniformProperty(std::vector<Property*>& properties, const std::string& name, VariantType type) {
	// name must be unique.
	for (int i = 0; i < properties.size(); ++i) {
		if (properties[i]->name == name) {
			return;
		}
	}

	Property* p = MEMORY_NEW(Property);
	p->name = name;
	switch (type) {
		case VariantTypeInt:
			p->value.SetInt(0);
			break;
		case VariantTypeFloat:
			p->value.SetFloat(0);
			break;
		case VariantTypeMatrix4:
			p->value.SetMatrix4(glm::mat4(0));
			break;
		case VariantTypeBool:
			p->value.SetBool(false);
			break;
		case VariantTypeVector3:
			if (String::EndsWith(name, "Color")) {
				p->value.SetColor3(glm::vec3(0));
			}
			else{
				p->value.SetVector3(glm::vec3(0));
			}
			break;
		case VariantTypeVector4:
			if (String::EndsWith(name, "Color")) {
				p->value.SetColor4(glm::vec4(0));
			}
			else {
				p->value.SetVector4(glm::vec4(0));
			}
			break;
		case VariantTypeTexture:
			p->value.SetTexture(nullptr);
			break;
	}

	properties.push_back(p);
}

void Pass::AddUniform(const char* name, GLenum type, GLuint location, GLint size) {
	Uniform* uniform = uniforms_[name];
	uniform->size = size;
	uniform->location = location;

	switch (type) {
	case GL_INT:
		uniform->type = VariantTypeInt;
		break;
	case GL_FLOAT:
		uniform->type = VariantTypeFloat;
		break;
	case GL_FLOAT_MAT4:
		uniform->type = (size == 1) ? VariantTypeMatrix4 : VariantTypeMatrix4Array;
		break;
	case GL_BOOL:
		uniform->type = VariantTypeBool;
		break;
	case GL_FLOAT_VEC3:
		uniform->type = VariantTypeVector3;
		break;
	case GL_FLOAT_VEC4:
		uniform->type = VariantTypeVector4;
		break;
	default:
		if (IsSampler(type)) {
			if (textureUnitCount_ >= GLUtils::GetLimits(GLLimitsMaxTextureUnits)) {
				Debug::LogError("too many textures.");
			}
			else {
				++textureUnitCount_;
				uniform->type = VariantTypeTexture;
			}
		}
		else {
			Debug::LogError("undefined uniform type 0x%x.", type);
		}

		break;
	}
}

void Pass::SetUniform(Uniform* uniform, const void* data) {
	SetUniform(uniform->location, uniform->type, uniform->size, data);
}

void Pass::SetUniform(GLuint location, VariantType type, uint size, const void* data) {
	switch (type) {
	case VariantTypeInt:
	case VariantTypeBool:
	case VariantTypeTexture:
		GL::ProgramUniform1iv(program_, location, size, (const GLint*)data);
		break;
	case VariantTypeFloat:
		GL::ProgramUniform1fv(program_, location, size, (const GLfloat*)data);
		break;
	case VariantTypeMatrix4:
	case VariantTypeMatrix4Array:
		GL::ProgramUniformMatrix4fv(program_, location, size, false, (const GLfloat*)data);
		break;
	case VariantTypeVector3:
		GL::ProgramUniform3fv(program_, location, size, (const GLfloat*)data);
		break;
	case VariantTypeVector4:
		GL::ProgramUniform4fv(program_, location, size, (const GLfloat*)data);
		break;
	default:
		Debug::LogError("unable to set uniform (type 0x%x).", type);
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

SubShader::SubShader() : passes_(nullptr), passCount_(0)
	, currentPass_(UINT_MAX), passEnabled_(UINT_MAX) {
	tags_[TagKeyRenderQueue] = RenderQueueGeometry;
}

SubShader::~SubShader() {
	MEMORY_DELETE_ARRAY(passes_);
}

bool SubShader::Initialize(std::vector<Property*>& properties, const Semantics::SubShader& config, const std::string& path) {
	InitializeTags(config.tags);

	passCount_ = config.passes.size();
	passes_ = MEMORY_NEW_ARRAY(Pass, config.passes.size());

	for (uint i = 0; i < passCount_; ++i) {
		passes_[i].Initialize(properties, config.passes[i], path);
		if (!config.passes[i].enabled) {
			passEnabled_ &= ~(1 << i);
		}
	}

	return true;
}

void SubShader::Bind(uint pass) {
	VERIFY_INDEX(pass, passCount_, NOARG);
	passes_[pass].Bind();
	currentPass_ = pass;
}

void SubShader::Unbind() {
	VERIFY_INDEX(currentPass_, passCount_, NOARG);
	passes_[currentPass_].Unbind();
	currentPass_ = UINT_MAX;
}

bool SubShader::IsPassEnabled(uint pass) const {
	VERIFY_INDEX(pass, passCount_, false);
	return (passEnabled_ & (1 << pass)) != 0;
}

int SubShader::GetPassIndex(const std::string& name) const {
	for (int i = 0; i < passCount_; ++i) {
		if (passes_[i].GetName() == name) {
			return i;
		}
	}

	return -1;
}

Pass* SubShader::GetPass(uint pass) {
	VERIFY_INDEX(pass, passCount_, nullptr);
	return passes_ + pass;
}

const Pass* SubShader::GetPass(uint pass) const {
	VERIFY_INDEX(pass, passCount_, nullptr);
	return passes_ + pass;
}

uint SubShader::GetNativePointer(uint pass) const {
	const Pass* p = GetPass(pass);
	if (p == nullptr) { return 0; }
	return p->GetNativePointer();
}

void SubShader::InitializeTags(const std::vector<Semantics::Tag>& tags) {
	for (uint i = 0; i < tags.size(); ++i) {
		InitializeTag(tags[i], i);
	}
}

void SubShader::InitializeTag(const Semantics::Tag& tag, uint i) {
	if (tag.key == "Queue") {
		tags_[TagKeyRenderQueue] = ParseExpression(TagKeyRenderQueue, tag.value);
	}
	else {
		Debug::LogError("invalid tag key %s",  tag.key.c_str());
	}
}

uint SubShader::ParseExpression(TagKey key, const std::string& expression) {
	if (key == TagKeyRenderQueue) {
		return (uint)GLEF::instance()->Evaluate(expression.c_str(), &renderQueueVariables);
	}

	Debug::LogError("invalid tag key %d.", key);
	return 0;
}

ShaderInternal::ShaderInternal() : ObjectInternal(ObjectTypeShader)
	, subShaderCount_(0), subShaders_(nullptr)
	, propertyCount_(0), properties_(nullptr), currentSubShader_(UINT_MAX) {
}

ShaderInternal::~ShaderInternal() {
	MEMORY_DELETE_ARRAY(subShaders_);
	ReleaseProperties();
}

std::string ShaderInternal::GetName() const {
	return FileSystem::GetFileNameWithoutExtension(path_);
}

bool ShaderInternal::Load(const std::string& path) {
	Semantics semantics;
	ShaderParser parser;
	if (!parser.Parse(semantics, path + GLSL_POSTFIX, "")) {
		return false;
	}

	ParseSubShaders(semantics.properties, semantics.subShaders, path);

	LoadProperties(semantics.properties);

	UniformBufferManager::instance()->Attach(SharedThis());

	path_ = path;
	return true;
}

void ShaderInternal::LoadProperties(const std::vector<Property*>& properties) {
	ReleaseProperties();
	properties_ = MEMORY_NEW_ARRAY(Property*, properties.size());
	propertyCount_ = properties.size();

	std::copy(properties.begin(), properties.end(), properties_);
}

void ShaderInternal::ParseSubShaders(std::vector<Property*>& properties, std::vector<Semantics::SubShader>& subShaders, const std::string& path) {
	subShaderCount_ = subShaders.size();
	subShaders_ = MEMORY_NEW_ARRAY(SubShader, subShaders.size());
	for (uint i = 0; i < subShaderCount_; ++i) {
		subShaders_[i].Initialize(properties, subShaders[i], path);
	}
}

void ShaderInternal::ReleaseProperties() {
	for (int i = 0; i < propertyCount_; ++i) {
		MEMORY_DELETE(properties_[i]);
	}

	MEMORY_DELETE_ARRAY(properties_);
	propertyCount_ = 0;
}

void ShaderInternal::Bind(uint ssi, uint pass) {
	VERIFY_INDEX(ssi, subShaderCount_, NOARG);
	subShaders_[ssi].Bind(pass);
	currentSubShader_ = ssi;
}

void ShaderInternal::Unbind() {
	VERIFY_INDEX(currentSubShader_, subShaderCount_, NOARG);
	subShaders_[currentSubShader_].Unbind();
	currentSubShader_ = UINT_MAX;
}

void ShaderInternal::SetRenderQueue(uint ssi, uint value) {
	VERIFY_INDEX(ssi, subShaderCount_, NOARG);
	return subShaders_[ssi].SetRenderQueue(value);
}

uint ShaderInternal::GetRenderQueue(uint ssi) const {
	VERIFY_INDEX(ssi, subShaderCount_, 0);
	return subShaders_[ssi].GetRenderQueue();
}

bool ShaderInternal::IsPassEnabled(uint ssi, uint pass) const {
	VERIFY_INDEX(ssi, subShaderCount_, false);
	return subShaders_[ssi].IsPassEnabled(pass);
}

uint ShaderInternal::GetNativePointer(uint ssi, uint pass) const {
	VERIFY_INDEX(ssi, subShaderCount_, 0);
	return subShaders_[ssi].GetNativePointer(pass);
}

int ShaderInternal::GetPassIndex(uint ssi, const std::string & name) const {
	VERIFY_INDEX(ssi, subShaderCount_, -1);
	return subShaders_[ssi].GetPassIndex(name);
}

void ShaderInternal::GetProperties(std::vector<const Property*>& properties) {
	properties.assign(properties_, properties_ + propertyCount_);
}

bool ShaderInternal::SetProperty(uint ssi, uint pass, const std::string& name, const void* data) {
	VERIFY_INDEX(ssi, subShaderCount_, false);
	return subShaders_[ssi].GetPass(pass)->SetProperty(name, data);
}
