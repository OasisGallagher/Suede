#include "shaderinternal.h"

#include "glef.h"

#include "math/mathf.h"
#include "vertexattrib.h"
#include "tools/string.h"
#include "os/filesystem.h"
#include "builtinproperties.h"

#include "internal/base/context.h"
#include "internal/base/renderdefines.h"

Shader::Shader() : Object(new ShaderInternal(Context::GetCurrent())) {}
std::string Shader::GetName() const { return _suede_dptr()->GetName(); }
void Shader::Bind(uint ssi, uint pass) { _suede_dptr()->Bind(ssi, pass); }
void Shader::Unbind() { _suede_dptr()->Unbind(); }
void Shader::SetRenderQueue(uint ssi, int value) { return _suede_dptr()->SetRenderQueue(ssi, value); }
int Shader::GetRenderQueue(uint ssi) const { return _suede_dptr()->GetRenderQueue(ssi); }
bool Shader::IsPassEnabled(uint ssi, uint pass) const { return _suede_dptr()->IsPassEnabled(ssi, pass); }
int Shader::GetPassIndex(uint ssi, const std::string& name) const { return _suede_dptr()->GetPassIndex(ssi, name); }
uint Shader::GetNativePointer(uint ssi, uint pass) const { return _suede_dptr()->GetNativePointer(ssi, pass); }
uint Shader::GetPassCount(uint ssi) const { return _suede_dptr()->GetPassCount(ssi); }
uint Shader::GetSubShaderCount() const { return _suede_dptr()->GetSubShaderCount(); }
void Shader::GetProperties(std::vector<ShaderProperty>& properties) { return _suede_dptr()->GetProperties(properties); }
bool Shader::SetProperty(uint ssi, uint pass, const std::string& name, const void* data) { return _suede_dptr()->SetProperty(ssi, pass, name, data); }

static GLEF glef;
static bool glefInitialized = false;
static std::mutex shaderMutex;
static std::map<std::string, ref_ptr<Shader>> shaderCache;

Shader* Shader::Find(const std::string& path) {
	auto ite = shaderCache.find(path);
	if (ite != shaderCache.end()) {
		return ite->second.get();
	}

	Shader* shader = new Shader();

	std::lock_guard<std::mutex> lock(shaderMutex);
	if (!_suede_doptr(shader)->Load(shader, path)) {
		delete shader;
		shader = nullptr;
	}

	shaderCache.insert(std::make_pair(path, shader));
	return shader;
}

static std::map<std::string, float> renderQueueVariables({
	std::make_pair("Background", (float)RenderQueue::Background),
	std::make_pair("Geometry", (float)RenderQueue::Geometry),
	std::make_pair("Transparent", (float)RenderQueue::Transparent),
	std::make_pair("Overlay", (float)RenderQueue::Overlay),
});

Pass::Pass(Context* context, const std::string& path) : context_(context), path_(path), program_(0), oldProgram_(0) {
	std::fill(states_, states_ + SUEDE_COUNTOF(states_), nullptr);
	std::fill(shaderObjs_, shaderObjs_ + ShaderStageCount, 0);
}

Pass::~Pass() {
	for (int i = 0; i < SUEDE_COUNTOF(states_); ++i) {
		delete states_[i];
	}

	Destroy();
}

bool Pass::Apply(std::vector<Property*>& properties, const Semantics::Pass& pass) {
	name_ = pass.name;
	if (program_ == 0) { program_ = context_->CreateProgram(); }

	InitializeRenderStates(pass.renderStates);
	
	std::string sources[ShaderStageCount];

	GLSLParser parser;
	if (!parser.Parse(sources, path_, pass.source, pass.lineno, "")) {
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
	context_->GetIntegerv(GL_CURRENT_PROGRAM, (int*)&oldProgram_);

	if (oldProgram_ != program_) {
		context_->UseProgram(program_);
	}
	else {
		oldProgram_ = -1;
	}
}

void Pass::Unbind() {
	UnbindRenderStates();
	if(oldProgram_ != -1) {
		context_->UseProgram(oldProgram_);
	}
}

uint Pass::GetNativePointer() const {
	SUEDE_ASSERT(program_ != 0);
	return program_;
}

void Pass::Destroy() {
	context_->DeleteProgram(program_);
	ClearIntermediateShaders();
}

void Pass::InitializeRenderStates(const std::vector<Semantics::RenderState>& states) {
	for (uint i = 0; i < states.size(); ++i) {
		RenderState* state = CreateRenderState(states[i]);
		delete states_[state->GetType()];
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

bool Pass::GetErrorMessage(uint shaderObj, std::string& answer) {
	int length = 0, writen = 0;
	context_->GetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &length);
	if (length > 1) {
		answer.resize(length);
		context_->GetShaderInfoLog(shaderObj, length, &writen, &answer[0]);
		answer.resize(writen);
		return true;
	}

	return false;
}

bool Pass::Link() {
	context_->LinkProgram(program_);

	int status = GL_FALSE;
	context_->GetProgramiv(program_, GL_LINK_STATUS, &status);

	if (status != GL_TRUE) {
		Debug::LogError("failed to link shader %s.", path_.c_str());
		return false;
	}

	context_->ValidateProgram(program_);
	context_->GetProgramiv(program_, GL_VALIDATE_STATUS, &status);
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
#define CASE(name)	if (state.type == #name) return new name ## State(context_)
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
			context_->DeleteShader(shaderObjs_[i]);
			shaderObjs_[i] = 0;
		}
	}
}

std::string Pass::LoadSource(ShaderStage stage, const char* source) {
	uint shaderObj = context_->CreateShader(StageInfo::At(stage).glShaderStage);

	context_->ShaderSource(shaderObj, 1, &source, nullptr);
	context_->CompileShader(shaderObj);

	context_->AttachShader(program_, shaderObj);

	std::string message;
	if (!GetErrorMessage(shaderObj, message)) {
		if (shaderObjs_[stage] != 0) {
			context_->DeleteShader(shaderObjs_[stage]);
		}

		shaderObjs_[stage] = shaderObj;
	}

	return message;
}

void Pass::UpdateVertexAttributes() {
	context_->BindAttribLocation(program_, VertexAttribPosition, "_Pos");

	for (int i = 0; i < Geometry::TexCoordsCount; ++i) {
		context_->BindAttribLocation(program_, VertexAttribTexCoord0 + i, ("_TexCoord" + std::to_string(i)).c_str());
	}

	context_->BindAttribLocation(program_, VertexAttribNormal, "_Normal");
	context_->BindAttribLocation(program_, VertexAttribTangent, "_Tangent");
	context_->BindAttribLocation(program_, VertexAttribBoneIndexes, "_BoneIndexes");
	context_->BindAttribLocation(program_, VertexAttribBoneWeights, "_BoneWeights");

	context_->BindAttribLocation(program_, VertexAttribInstanceColor, "_InstanceColor");
	context_->BindAttribLocation(program_, VertexAttribInstanceGeometry, "_InstanceGeometry");
}

void Pass::UpdateFragmentAttributes() {
}

void Pass::AddAllUniforms() {
	uniforms_.clear();
	textureUnitCount_ = 0;

	uint type;
	uint location = 0;
	int size, count, maxLength, length;

	context_->GetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);
	context_->GetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

	char* name = new char[maxLength];
	for (int i = 0; i < count; ++i) {
		context_->GetActiveUniform(program_, i, maxLength, &length, &size, &type, name);

		location = context_->GetUniformLocation(program_, name);

		// -1 indicates that is not an active uniform, although it may be present in a
		// uniform block.
		if (location == GL_INVALID_INDEX) {
			continue;
		}

		char* ptr = strrchr(name, '[');
		if (ptr != nullptr) { *ptr = 0; }

		AddUniform(name, type, location, size);
	}

	delete[] name;
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

	Property* p = new Property;
	p->name = name;
	switch (type) {
		case VariantType::Int:
			p->value.SetInt(0);
			break;
		case VariantType::Float:
			p->value.SetFloat(0);
			break;
		case VariantType::Matrix4:
			p->value.SetMatrix4(Matrix4(0));
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
				p->value.SetVector3(Vector3(0));
			}
			else {
				p->value.SetVector4(Vector4(0));
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

void Pass::AddUniform(const char* name, uint type, uint location, int size) {
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
			int limit = context_->GetLimit(ContextLimitType::MaxCombinedTextureImageUnits);
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

void Pass::SetUniform(uint location, VariantType type, uint size, const void* data) {
	switch (type) {
	case VariantType::Int:
	case VariantType::Bool:
	case VariantType::Texture:
		context_->ProgramUniform1iv(program_, location, size, (const int*)data);
		break;
	case VariantType::Float:
		context_->ProgramUniform1fv(program_, location, size, (const float*)data);
		break;
	case VariantType::Matrix4:
	case VariantType::Matrix4Array:
		context_->ProgramUniformMatrix4fv(program_, location, size, false, (const float*)data);
		break;
	case VariantType::Vector3:
		context_->ProgramUniform3fv(program_, location, size, (const float*)data);
		break;
	case VariantType::Vector4:
		context_->ProgramUniform4fv(program_, location, size, (const float*)data);
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

SubShader::SubShader(Context* context, const Semantics::SubShader& config, const std::string& path) : context_(context), passCount_(0)
	, currentPass_(-1), passEnabled_(UINT_MAX) {
	tags_[TagKeyRenderQueue] = (int)RenderQueue::Geometry;
	InitializeTags(config.tags_);

	passCount_ = config.passes.size();
	passes_.resize(config.passes.size(), context_, path);

	for (uint i = 0; i < passCount_; ++i) {
		if (!config.passes[i].enabled) {
			passEnabled_ &= ~(1 << i);
		}
	}
}

SubShader::~SubShader() {
	
}

bool SubShader::Apply(std::vector<ShaderProperty>& properties, const Semantics::SubShader& config) {
	std::vector<Property*> container;
	for (uint i = 0; i < passCount_; ++i) {
		passes_[i].Apply(container, config.passes[i]);

		AddShaderProperties(properties, container, i);
		container.clear();
	}

	return true;
}

void SubShader::Bind(uint pass) {
	SUEDE_ASSERT(pass < passCount_);
	passes_[pass].Bind();
	currentPass_ = pass;
}

void SubShader::Unbind() {
	SUEDE_ASSERT(currentPass_ < passCount_);
	passes_[currentPass_].Unbind();
	currentPass_ = UINT_MAX;
}

bool SubShader::IsPassEnabled(uint pass) const {
	SUEDE_ASSERT(pass < passCount_);
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
	return &passes_[pass];
}

const Pass* SubShader::GetPass(uint pass) const {
	return &passes_[pass];
}

uint SubShader::GetNativePointer(uint pass) const {
	const Pass* p = GetPass(pass);
	if (p == nullptr) { return 0; }
	return p->GetNativePointer();
}

void SubShader::InitializeTags(const std::vector<Semantics::Tag>& tags_) {
	for (uint i = 0; i < tags_.size(); ++i) {
		InitializeTag(tags_[i], i);
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
		return (uint)glef.Evaluate(expression.c_str(), &renderQueueVariables);
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

			delete p;
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

event<ShaderInternal*> ShaderInternal::shaderCreated;

ShaderInternal::ShaderInternal(Context* context) : ObjectInternal(ObjectType::Shader)
	, GLObjectMaintainer(context), currentSubShader_(UINT_MAX) {
}

ShaderInternal::~ShaderInternal() {
	ReleaseProperties();
	ReleaseSubShaders();
}

std::string ShaderInternal::GetName() const {
	return FileSystem::GetFileNameWithoutExtension(path_);
}

bool ShaderInternal::Load(Shader* self, const std::string& path) {
	if (!glefInitialized) {
		glef.Load("resources/data/GLEF.dat");
		glefInitialized = true;
	}

	semantics_.reset(new Semantics());
	ShaderParser parser(&glef);
	if (!parser.Parse(*semantics_, path + GLSL_POSTFIX, "")) {
		return false;
	}

	std::vector<ShaderProperty> properties;

	ParseSemanticProperties(properties, *semantics_);
	ParseSubShader(properties, semantics_->subShaders, path);

	SetProperties(properties);

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
	subShaders_.resize(subShaders.size());
	for (uint i = 0; i < subShaders_.size(); ++i) {
		subShaders_[i] = new SubShader(context_, subShaders[i], path);
	}
}

void ShaderInternal::ReleaseSubShaders() {
	for (SubShader* p : subShaders_) {
		delete p;
	}

	subShaders_.clear();
}

void ShaderInternal::ReleaseProperties() {
	for (ShaderProperty& p : properties_) {
		delete p.property;
	}

	properties_.clear();
}

void ShaderInternal::OnContextDestroyed() {
	subShaders_.clear();
	GLObjectMaintainer::OnContextDestroyed();
}

void ShaderInternal::Bind(uint ssi, uint pass) {
	if (semantics_) {
		for (int i = 0; i < subShaders_.size(); ++i) {
			subShaders_[i]->Apply(properties_, semantics_->subShaders[i]);
		}

		semantics_ = nullptr;
		shaderCreated.raise(this);
	}

	subShaders_[ssi]->Bind(pass);
	currentSubShader_ = ssi;
}

void ShaderInternal::Unbind() {
	subShaders_[currentSubShader_]->Unbind();
	currentSubShader_ = UINT_MAX;
}

void ShaderInternal::SetRenderQueue(uint ssi, int value) {
	return subShaders_[ssi]->SetRenderQueue(value);
}

int ShaderInternal::GetRenderQueue(uint ssi) const {
	return subShaders_[ssi]->GetRenderQueue();
}

bool ShaderInternal::IsPassEnabled(uint ssi, uint pass) const {
	return subShaders_[ssi]->IsPassEnabled(pass);
}

uint ShaderInternal::GetNativePointer(uint ssi, uint pass) const {
	return subShaders_[ssi]->GetNativePointer(pass);
}

int ShaderInternal::GetPassIndex(uint ssi, const std::string & name) const {
	return subShaders_[ssi]->GetPassIndex(name);
}

void ShaderInternal::GetProperties(std::vector<ShaderProperty>& properties) {
	properties = properties_;
}

bool ShaderInternal::SetProperty(uint ssi, uint pass, const std::string& name, const void* data) {
	if (data == nullptr) { return false; }

	return subShaders_[ssi]->GetPass(pass)->SetProperty(name, data);
}
