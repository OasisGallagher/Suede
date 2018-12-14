#include "glef.h"
#include "tools/math2.h"
#include "vertexattrib.h"
#include "tools/string.h"
#include "os/filesystem.h"
#include "../api/glutils.h"
#include "shaderinternal.h"
#include "builtinproperties.h"

#include "internal/base/renderdefines.h"

#include "internal/rendering/uniformbuffermanager.h"

IShader::IShader() : IObject(MEMORY_NEW(ShaderInternal)) {}
std::string IShader::GetName() const { return _suede_dptr()->GetName(); }
bool IShader::Load(const std::string& path) { return _suede_dptr()->Load(_shared_this(), path); }
void IShader::Bind(uint ssi, uint pass) { _suede_dptr()->Bind(ssi, pass); }
void IShader::Unbind() { _suede_dptr()->Unbind(); }
void IShader::SetRenderQueue(uint ssi, int value) { return _suede_dptr()->SetRenderQueue(ssi, value); }
int IShader::GetRenderQueue(uint ssi) const { return _suede_dptr()->GetRenderQueue(ssi); }
bool IShader::IsPassEnabled(uint ssi, uint pass) const { return _suede_dptr()->IsPassEnabled(ssi, pass); }
int IShader::GetPassIndex(uint ssi, const std::string& name) const { return _suede_dptr()->GetPassIndex(ssi, name); }
uint IShader::GetNativePointer(uint ssi, uint pass) const { return _suede_dptr()->GetNativePointer(ssi, pass); }
uint IShader::GetPassCount(uint ssi) const { return _suede_dptr()->GetPassCount(ssi); }
uint IShader::GetSubShaderCount() const { return _suede_dptr()->GetSubShaderCount(); }
void IShader::GetProperties(std::vector<ShaderProperty>& properties) { return _suede_dptr()->GetProperties(properties); }
bool IShader::SetProperty(uint ssi, uint pass, const std::string& name, const void* data) { return _suede_dptr()->SetProperty(ssi, pass, name, data); }

std::pair<std::string, float> _variables[] = {
	std::make_pair("Background", (float)RenderQueue::Background),
	std::make_pair("Geometry", (float)RenderQueue::Geometry),
	std::make_pair("Transparent", (float)RenderQueue::Transparent),
	std::make_pair("Overlay", (float)RenderQueue::Overlay),
};

static std::map<std::string, float> renderQueueVariables(_variables, _variables + SUEDE_COUNTOF(_variables));

#define BIND(old, new)	if (old == new) { old = -1; } else
#define UNBIND(old)		if (old == -1) { } else

Pass::Pass() : program_(0), oldProgram_(0) {
	program_ = GL::CreateProgram();

	std::fill(states_, states_ + SUEDE_COUNTOF(states_), nullptr);
	std::fill(shaderObjs_, shaderObjs_ + ShaderStageCount, 0);
}

Pass::~Pass() {
	for (int i = 0; i < SUEDE_COUNTOF(states_); ++i) {
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
	if (!parser.Parse(sources, path, pass.source, pass.lineno, "")) {
		return false;
	}

	for (int i = 0; i < ShaderStageCount; ++i) {
		std::vector<std::string> lines;
		String::Split(lines, sources[i], '\n');

		std::string shaderError;
		if (!sources[i].empty() && !(shaderError = LoadSource((ShaderStage)i, sources[i].c_str())).empty()) {
			Debug::LogError(parser.TranslateErrorMessage((ShaderStage)i, shaderError).c_str());
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
	for (int i = 0; i < SUEDE_COUNTOF(states_); ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Bind();
		}
	}
}

void Pass::UnbindRenderStates() {
	for (int i = 0; i < SUEDE_COUNTOF(states_); ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Unbind();
		}
	}
}

bool Pass::GetErrorMessage(GLuint shaderObj, std::string& answer) {
	GLint length = 0, writen = 0;
	GL::GetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &length);
	if (length > 1) {
		answer.resize(length);
		GL::GetShaderInfoLog(shaderObj, length, &writen, &answer[0]);
		answer.resize(writen);
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
		answer = RenderStateParameter::None;
		return true;
	}

#define CASE(value)	if (parameter == #value) { answer = RenderStateParameter::value; return true; } else (void)0
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

std::string Pass::LoadSource(ShaderStage stage, const char* source) {
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
	}

	return message;
}

void Pass::UpdateVertexAttributes() {
	GL::BindAttribLocation(program_, VertexAttribPosition, "_Pos");

	for (int i = 0; i < MeshAttribute::TexCoordsCount; ++i) {
		GL::BindAttribLocation(program_, VertexAttribTexCoord0 + i, ("_TexCoord" + std::to_string(i)).c_str());
	}

	GL::BindAttribLocation(program_, VertexAttribNormal, "_Normal");
	GL::BindAttribLocation(program_, VertexAttribTangent, "_Tangent");
	GL::BindAttribLocation(program_, VertexAttribBoneIndexes, "_BoneIndexes");
	GL::BindAttribLocation(program_, VertexAttribBoneWeights, "_BoneWeights");

	GL::BindAttribLocation(program_, VertexAttribInstanceColor, "_InstanceColor");
	GL::BindAttribLocation(program_, VertexAttribInstanceGeometry, "_InstanceGeometry");
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
		case VariantType::Int:
			p->value.SetInt(0);
			break;
		case VariantType::Float:
			p->value.SetFloat(0);
			break;
		case VariantType::Matrix4:
			p->value.SetMatrix4(glm::mat4(0));
			break;
		case VariantType::Matrix4Array:
			p->value.SetMatrix4Array(nullptr, 0);
			break;
		case VariantType::Bool:
			p->value.SetBool(false);
			break;
		case VariantType::Vector3:
		case VariantType::Vector4:
			if (BuiltinProperties::IsBuiltinColorProperty(name.c_str())) {
				p->value.SetColor(Color::black);
			}
			else if(type == VariantType::Vector3) {
				p->value.SetVector3(glm::vec3(0));
			}
			else {
				p->value.SetVector4(glm::vec4(0));
			}
			break;
		case VariantType::Texture:
			p->value.SetTexture(nullptr);
			break;
		default:
			Debug::LogError("unknown uniform type %s.", type.to_string());
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
		uniform->type = VariantType::Int;
		break;
	case GL_FLOAT:
		uniform->type = VariantType::Float;
		break;
	case GL_FLOAT_MAT4:
		uniform->type = (size == 1) ? VariantType::Matrix4 : VariantType::Matrix4Array;
		break;
	case GL_BOOL:
		uniform->type = VariantType::Bool;
		break;
	case GL_FLOAT_VEC3:
		uniform->type = VariantType::Vector3;
		break;
	case GL_FLOAT_VEC4:
		uniform->type = VariantType::Vector4;
		break;
	default:
		if (IsSampler(type)) {
			int limit = GLUtils::GetLimits(GLLimitsMaxCombinedTextureImageUnits);
			if (textureUnitCount_ >= limit) {
				Debug::LogError("too many textures. limit is %d.", limit);
			}
			else {
				++textureUnitCount_;
				uniform->type = VariantType::Texture;
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
	case VariantType::Int:
	case VariantType::Bool:
	case VariantType::Texture:
		GL::ProgramUniform1iv(program_, location, size, (const GLint*)data);
		break;
	case VariantType::Float:
		GL::ProgramUniform1fv(program_, location, size, (const GLfloat*)data);
		break;
	case VariantType::Matrix4:
	case VariantType::Matrix4Array:
		GL::ProgramUniformMatrix4fv(program_, location, size, false, (const GLfloat*)data);
		break;
	case VariantType::Vector3:
		GL::ProgramUniform3fv(program_, location, size, (const GLfloat*)data);
		break;
	case VariantType::Vector4:
		GL::ProgramUniform4fv(program_, location, size, (const GLfloat*)data);
		break;
	default:
		Debug::LogError("unable to set uniform (type %s).", type.to_string());
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
	tags_[TagKeyRenderQueue] = (int)RenderQueue::Geometry;
}

SubShader::~SubShader() {
	MEMORY_DELETE_ARRAY(passes_);
}

bool SubShader::Initialize(std::vector<ShaderProperty>& properties, const Semantics::SubShader& config, const std::string& path) {
	InitializeTags(config.tags);

	passCount_ = config.passes.size();
	passes_ = MEMORY_NEW_ARRAY(Pass, config.passes.size());

	std::vector<Property*> container;
	for (uint i = 0; i < passCount_; ++i) {
		passes_[i].Initialize(container, config.passes[i], path);
		if (!config.passes[i].enabled) {
			passEnabled_ &= ~(1 << i);
		}

		AddShaderProperties(properties, container, i);
		container.clear();
	}

	return true;
}

void SubShader::Bind(uint pass) {
	SUEDE_VERIFY_INDEX(pass, passCount_, SUEDE_NOARG);
	passes_[pass].Bind();
	currentPass_ = pass;
}

void SubShader::Unbind() {
	SUEDE_VERIFY_INDEX(currentPass_, passCount_, SUEDE_NOARG);
	passes_[currentPass_].Unbind();
	currentPass_ = UINT_MAX;
}

bool SubShader::IsPassEnabled(uint pass) const {
	SUEDE_VERIFY_INDEX(pass, passCount_, false);
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
	SUEDE_VERIFY_INDEX(pass, passCount_, nullptr);
	return passes_ + pass;
}

const Pass* SubShader::GetPass(uint pass) const {
	SUEDE_VERIFY_INDEX(pass, passCount_, nullptr);
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

void SubShader::AddShaderProperties(std::vector<ShaderProperty>& properties, const std::vector<Property*> container, uint pass) {
	for (Property* p : container) {
		ShaderProperty* target = nullptr;
		for (ShaderProperty& sp : properties) {
			if (sp.property->name == p->name) {
				target = &sp;
				break;
			}
		}

		if (target == nullptr) {
			ShaderProperty sp = { 1 << pass, p };
			properties.push_back(sp);
		}
		else {
			if (CheckPropertyCompatible(target, p)) {
				target->mask |= (1 << pass);
			}

			MEMORY_DELETE(p);
		}
	}
}

bool SubShader::CheckPropertyCompatible(ShaderProperty* target, Property* p) {
	VariantType lhs = target->property->value.GetType();
	VariantType rhs = p->value.GetType();
	if (lhs == rhs 
		|| (lhs == VariantType::Color && (rhs == VariantType::Vector3 || rhs == VariantType::Vector4))
		|| (lhs == VariantType::RangedInt && rhs == VariantType::Int)
		|| (lhs == VariantType::RangedFloat && rhs == VariantType::Float)) {
		return true;
	}

	Debug::LogError("type of property %s %s is incompatible with %s.", p->name.c_str(), lhs.to_string(), rhs.to_string());
	return false;
}

ShaderInternal::ShaderInternal() : ObjectInternal(ObjectType::Shader)
	, subShaderCount_(0), subShaders_(nullptr), currentSubShader_(UINT_MAX) {
}

ShaderInternal::~ShaderInternal() {
	MEMORY_DELETE_ARRAY(subShaders_);
	ReleaseProperties();
}

std::string ShaderInternal::GetName() const {
	return FileSystem::GetFileNameWithoutExtension(path_);
}

bool ShaderInternal::Load(Shader self, const std::string& path) {
	Semantics semantics;
	ShaderParser parser;
	if (!parser.Parse(semantics, path + GLSL_POSTFIX, "")) {
		return false;
	}

	std::vector<ShaderProperty> properties;

	ParseSemanticProperties(properties, semantics);
	ParseSubShader(properties, semantics.subShaders, path);

	SetProperties(properties);

	UniformBufferManager::instance()->Attach(self);

	path_ = path;
	return true;
}

void ShaderInternal::SetProperties(const std::vector<ShaderProperty>& properties) {
	ReleaseProperties();
	properties_ = properties;
}

void ShaderInternal::ParseSemanticProperties(std::vector<ShaderProperty>& properties, const Semantics& semantics) {
	for (Property* p : semantics.properties) {
		ShaderProperty sp = { -1, p };
		properties.push_back(sp);
	}
}

void ShaderInternal::ParseSubShader(std::vector<ShaderProperty>& properties, const std::vector<Semantics::SubShader>& subShaders, const std::string& path) {
	subShaderCount_ = subShaders.size();
	subShaders_ = MEMORY_NEW_ARRAY(SubShader, subShaders.size());
	for (uint i = 0; i < subShaderCount_; ++i) {
		subShaders_[i].Initialize(properties, subShaders[i], path);
	}
}

void ShaderInternal::ReleaseProperties() {
	for (ShaderProperty& p : properties_) {
		MEMORY_DELETE(p.property);
	}

	properties_.clear();
}

void ShaderInternal::Bind(uint ssi, uint pass) {
	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, SUEDE_NOARG);
	subShaders_[ssi].Bind(pass);
	currentSubShader_ = ssi;
}

void ShaderInternal::Unbind() {
	SUEDE_VERIFY_INDEX(currentSubShader_, subShaderCount_, SUEDE_NOARG);
	subShaders_[currentSubShader_].Unbind();
	currentSubShader_ = UINT_MAX;
}

void ShaderInternal::SetRenderQueue(uint ssi, int value) {
	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, SUEDE_NOARG);
	return subShaders_[ssi].SetRenderQueue(value);
}

int ShaderInternal::GetRenderQueue(uint ssi) const {
	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, 0);
	return subShaders_[ssi].GetRenderQueue();
}

bool ShaderInternal::IsPassEnabled(uint ssi, uint pass) const {
	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, false);
	return subShaders_[ssi].IsPassEnabled(pass);
}

uint ShaderInternal::GetNativePointer(uint ssi, uint pass) const {
	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, 0);
	return subShaders_[ssi].GetNativePointer(pass);
}

int ShaderInternal::GetPassIndex(uint ssi, const std::string & name) const {
	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, -1);
	return subShaders_[ssi].GetPassIndex(name);
}

void ShaderInternal::GetProperties(std::vector<ShaderProperty>& properties) {
	properties = properties_;
}

bool ShaderInternal::SetProperty(uint ssi, uint pass, const std::string& name, const void* data) {
	if (data == nullptr) { return false; }

	SUEDE_VERIFY_INDEX(ssi, subShaderCount_, false);
	return subShaders_[ssi].GetPass(pass)->SetProperty(name, data);
}
