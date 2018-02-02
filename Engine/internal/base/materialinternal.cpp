#include "variables.h"
#include "tools/math2.h"
#include "renderstate.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "materialinternal.h"

// TODO: sub shader index.
#define SUB_SHADER_INDEX	0

MaterialInternal::MaterialInternal()
	: ObjectInternal(ObjectTypeMaterial), pass_(-1) {
}

MaterialInternal::~MaterialInternal() {
}

Object MaterialInternal::Clone() {
	Material clone = NewMaterial();
	MaterialInternal* clonePtr = dynamic_cast<MaterialInternal*>(clone.get());
	*clonePtr = *this;

	return clone;
}

void MaterialInternal::SetShader(Shader value) {
	if (shader_) {
		// TODO: unbind properties ?
//		UnbindProperties();
//		properties_.clear();
		passEnabled_ = 0;
	}

	shader_ = value;

	InitializeProperties();
	InitializeEnabledState();
}

void MaterialInternal::SetInt(const std::string& name, int value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeInt);
	//if (var != nullptr && var->GetInt() != value) {
		var->SetInt(value);
	//}
}

void MaterialInternal::SetFloat(const std::string& name, float value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeFloat);
	//if (var != nullptr && !Math::Approximately(var->GetFloat(), value)) {
		var->SetFloat(value);
	//}
}

void MaterialInternal::SetTexture(const std::string& name, Texture value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeTexture);
	//if (var != nullptr && var->GetTexture() != value) {
		var->SetTexture(value);
	//}
}

void MaterialInternal::SetVector3(const std::string& name, const glm::vec3& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeVector3);
	//if (var != nullptr && var->GetVector3() != value) {
		var->SetVector3(value);
	//}
}

void MaterialInternal::SetVector4(const std::string& name, const glm::vec4& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeVector4);
	//if (var != nullptr && var->GetVector4() != value) {
		var->SetVector4(value);
	//}
}

void MaterialInternal::SetMatrix4(const std::string& name, const glm::mat4& value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeMatrix4);
	//if (var != nullptr && var->GetMatrix4() != value) {
		var->SetMatrix4(value);
	//}
}

void MaterialInternal::SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	Variant* var = GetProperty(name, VariantTypeMatrix4Array);
	//if (var != nullptr) {
		var->SetMatrix4Array(ptr, count);
	//}
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

void MaterialInternal::GetProperties(std::vector<Property>& properties) const {
	for (PropertyContainer::const_iterator ite = properties_.cbegin(); ite != properties_.cend(); ++ite) {
		Property value = { ite->first, *ite->second };
		properties.push_back(value);
	}
}

uint MaterialInternal::GetPassCount() const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	return shader_->GetPassCount(SUB_SHADER_INDEX);
}

void MaterialInternal::Bind(uint pass) {
	if (!IsPassEnabled(pass)) {
		Debug::LogError("pass %d is not enabled.", pass);
		return;
	}

	BindProperties(pass);
	shader_->Bind(SUB_SHADER_INDEX, pass);
}

void MaterialInternal::Unbind() {
	UnbindProperties();
	shader_->Unbind();
}

bool MaterialInternal::EnablePass(uint pass) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return false;
	}

	passEnabled_ |= (1 << pass);
	return true;
}

bool MaterialInternal::DisablePass(uint pass) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return false;
	}

	passEnabled_ &= ~(1 << pass);
	return true;
}

bool MaterialInternal::IsPassEnabled(uint pass) const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return false;
	}

	return (passEnabled_ & (1 << pass)) != 0;
}

int MaterialInternal::GetPassIndex(const std::string& name) const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return -1;
	}

	return shader_->GetPassIndex(SUB_SHADER_INDEX, name);
}

void MaterialInternal::Define(const std::string& name) {
}

void MaterialInternal::Undefine(const std::string& name) {
}

Variant* MaterialInternal::GetProperty(const std::string& name, VariantType type) {
	return properties_[name];
	/*
	Variant* ans = nullptr;
	if (!properties_.get(name, ans)) {
		if (!String::StartsWith(name, VARIABLE_PREFIX)) {
			Debug::LogWarning("property %s does not exist.", name.c_str());
		}

		return false;
	}

	if (ans->GetType() != type) {
		Debug::LogError("property %s does not defined as %s.", name.c_str(), Variant::TypeString(type));
		return false;
	}

	return ans;
	*/
}

void MaterialInternal::BindProperties(uint pass) {
	int textureIndex = 0;
	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		Variant* var = ite->second;
		if (var->GetType() != VariantTypeTexture) {
			shader_->SetProperty(SUB_SHADER_INDEX, pass, ite->first, var->GetData());
		}
		else if (var->GetTexture()) {
			shader_->SetProperty(SUB_SHADER_INDEX, pass, ite->first, &textureIndex);
			var->GetTexture()->Bind(textureIndex++);
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

void MaterialInternal::InitializeProperties() {
	std::vector<Property> container;
	shader_->GetProperties(container);

	//	properties_.clear();
	for (int i = 0; i < container.size(); ++i) {
		*properties_[container[i].name] = container[i].value;
	}
}

void MaterialInternal::InitializeEnabledState() {
	passEnabled_ = UINT_MAX;
	for (int i = 0; i < shader_->GetPassCount(SUB_SHADER_INDEX); ++i) {
		if (!shader_->IsPassEnabled(SUB_SHADER_INDEX, i)) {
			passEnabled_ &= ~(1 << i);
		}
	}
}
