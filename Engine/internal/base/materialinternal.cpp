#include "debug.h"
#include "math2.h"
#include "variables.h"
#include "renderstate.h"
#include "tools/string.h"
#include "materialinternal.h"

MaterialInternal::MaterialInternal()
	: ObjectInternal(ObjectTypeMaterial) , oldProgram_(0) {
	std::fill(states_, states_ + RenderStateCount, nullptr);
}

MaterialInternal::~MaterialInternal() {
	for (int i = 0; i < RenderStateCount; ++i) {
		MEMORY_RELEASE(states_[i]);
	}
}

Object MaterialInternal::Clone() {
	Material clone = NewMaterial();
	MaterialInternal* clonePtr = dynamic_cast<MaterialInternal*>(clone.get());
	*clonePtr = *this;

	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			clonePtr->states_[i] = states_[i]->Clone();
		}
	}
	
	return clone;
}

void MaterialInternal::SetShader(Shader value) {
	if (shader_) {
		UnbindProperties();
	}

	shader_ = value;

	std::vector<ShaderProperty> container;
	shader_->GetProperties(container);

	properties_.clear();
	int textureCount = 0;
	for (int i = 0; i < container.size(); ++i) {
		Variant* var = properties_[container[i].name];
		switch (container[i].type) {
			case ShaderPropertyTypeInt:
				var->SetInt(0);
			case ShaderPropertyTypeBool:
				var->SetBool(false);
				break;
			case ShaderPropertyTypeFloat:
				var->SetFloat(0);
				break;
			case ShaderPropertyTypeMatrix4:
				var->SetMatrix4(glm::mat4(1));
				break;
			case ShaderPropertyTypeMatrix4Array:
				var->SetPodBuffer(nullptr, 0);
				break;
			case ShaderPropertyTypeVector3:
				var->SetVector3(glm::vec3(0));
				break;
			case ShaderPropertyTypeVector4:
				var->SetVector4(glm::vec4(0, 0, 0, 1));
				break;
			case ShaderPropertyTypeTexture:
				var->SetTextureIndex(textureCount++);
				break;
			default:
				Debug::LogError("invalid property type %d.", container[i].type);
				break;
		}
	}
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

	Variant* var = GetProperty(name, VariantTypeInt);
	if (var != nullptr && var->GetInt() != value) {
		var->SetInt(value);
	}
}

void MaterialInternal::SetFloat(const std::string& name, float value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeFloat);
	if (var != nullptr && !Math::Approximately(var->GetFloat(), value)) {
		var->SetFloat(value);
	}
}

void MaterialInternal::SetTexture(const std::string& name, Texture value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeTexture);

	if (var != nullptr && var->GetTexture() != value) {
		var->SetTexture(value);
	}
}

void MaterialInternal::SetVector3(const std::string& name, const glm::vec3& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeVector3);
	if (var != nullptr && var->GetVector3() != value) {
		var->SetVector3(value);
	}
}

void MaterialInternal::SetVector4(const std::string& name, const glm::vec4& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeVector4);
	if (var != nullptr && var->GetVector4() != value) {
		var->SetVector4(value);
	}
}

void MaterialInternal::SetMatrix4(const std::string& name, const glm::mat4& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeMatrix4);
	if (var != nullptr && var->GetMatrix4() != value) {
		var->SetMatrix4(value);
	}
}

void MaterialInternal::SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypePodBuffer);
	if (var != nullptr) {
		var->SetPodBuffer(ptr, count * sizeof(glm::mat4));
	}
}

int MaterialInternal::GetInt(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	Variant* var = GetProperty(name, VariantTypeInt);
	if (var == nullptr) {
		return 0;
	}

	return var->GetInt();
}

float MaterialInternal::GetFloat(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	Variant* var = GetProperty(name, VariantTypeFloat);
	if (var == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return 0.f;
	}

	return var->GetFloat();
}

Texture MaterialInternal::GetTexture(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return nullptr;
	}

	Variant* var = GetProperty(name, VariantTypeTexture);
	if (var == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return Texture();
	}

	return var->GetTexture();
}

glm::mat4 MaterialInternal::GetMatrix4(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return glm::mat4(0);
	}

	Variant* var = GetProperty(name, VariantTypeMatrix4);
	if (var == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return glm::mat4(0);
	}

	return var->GetMatrix4();
}

glm::vec3 MaterialInternal::GetVector3(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return glm::vec3(0);
	}

	Variant* var = GetProperty(name, VariantTypeVector3);
	if (var == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return glm::vec3(0);
	}

	return var->GetVector3();
}

glm::vec4 MaterialInternal::GetVector4(const std::string& name) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return glm::vec4(0);
	}

	Variant* var = GetProperty(name, VariantTypeVector4);
	if (var == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
		return glm::vec4(0);
	}

	return var->GetVector4();
}

void MaterialInternal::Bind() {
	BindRenderStates();
	BindProperties();

	glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram_);
	glUseProgram(shader_->GetNativePointer());
}

void MaterialInternal::Unbind() {
	UnbindProperties();
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

Variant* MaterialInternal::GetProperty(const std::string& name, VariantType type) {
	Variant* ans = nullptr;
	if (!properties_.get(name, ans)) {
		static int variablePrefixLength = strlen(VARIABLE_PREFIX);
		if (strncmp(name.c_str(), VARIABLE_PREFIX, variablePrefixLength) != 0) {
			Debug::LogWarning("property %s does not exist.", name.c_str());
		}

		return false;
	}

	if (ans->GetType() != type) {
		Debug::LogError("property %s does not defined as %s.", name.c_str(), Variant::TypeString(type));
		return false;
	}

	return ans;
}

void MaterialInternal::BindProperties() {
	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		Variant* var = ite->second;
		if (var->GetType() != VariantTypeTexture) {
			shader_->SetProperty(ite->first, var->GetData());
		}
		else if(var->GetTexture()){
			var->GetTexture()->Bind(var->GetTextureIndex());
		}
	}
}

void MaterialInternal::UnbindProperties() {
	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		Variant* var = ite->second;
		if (var->GetType() != VariantTypeTexture) {
			// need clear uniform ?
			//shader_->SetProperty(ite->first, ???);
		}
		else if (var->GetTexture()) {
			var->GetTexture()->Unbind();
		}
	}
}

