#include "variables.h"
#include "tools/math2.h"
#include "renderstate.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "renderdefines.h"
#include "materialinternal.h"

#include "internal/rendering/sharedtexturemanager.h"

// TODO: sub shader index.
#define SUB_SHADER_INDEX	0

MaterialInternal::MaterialInternal()
	: ObjectInternal(ObjectTypeMaterial), currentPass_(-1), name_(UNNAMED_MATERIAL) {
}

MaterialInternal::~MaterialInternal() {
}

Object MaterialInternal::Clone() {
	Material clone = NewMaterial();
	MaterialInternal* clonePtr = InternalPtr(clone);
	*clonePtr = *this;

	return clone;
}

void MaterialInternal::SetShader(Shader value) {
	shader_ = value;

	InitializeProperties();
	InitializeEnabledState();
}

void MaterialInternal::SetInt(const std::string& name, int value) {
	Variant* var = GetProperty(name, VariantTypeInt);
	if (var != nullptr && var->GetInt() != value) {
		var->SetInt(value);
	}
}

void MaterialInternal::SetBool(const std::string& name, bool value) {
	Variant* var = GetProperty(name, VariantTypeBool);
	if (var != nullptr && var->GetBool() != value) {
		var->SetBool(value);
	}
}

void MaterialInternal::SetFloat(const std::string& name, float value) {
	Variant* var = GetProperty(name, VariantTypeFloat);
	if (var != nullptr && !Math::Approximately(var->GetFloat(), value)) {
		var->SetFloat(value);
	}
}

void MaterialInternal::SetTexture(const std::string& name, Texture value) {
	Variant* var = GetProperty(name, VariantTypeTexture);
	if (var != nullptr && var->GetTexture() != value) {
		var->SetTexture(value);
	}
}

void MaterialInternal::SetVector3(const std::string& name, const glm::vec3& value) {
	Variant* var = GetProperty(name, VariantTypeVector3);
	if (var != nullptr && var->GetVector3() != value) {
		var->SetVector3(value);
	}
}

void MaterialInternal::SetVector3Array(const std::string& name, const glm::vec3* ptr, uint count) {
	Variant* var = GetProperty(name, VariantTypeVector3Array);
	if (var != nullptr) {
		var->SetVector3Array(ptr, count);
	}
}

void MaterialInternal::SetColor3(const std::string& name, const glm::vec3& value) {
	Variant* var = GetProperty(name, VariantTypeColor3);
	if (var != nullptr && var->GetColor3() != value) {
		var->SetColor3(value);
	}
}

void MaterialInternal::SetColor4(const std::string& name, const glm::vec4& value) {
	Variant* var = GetProperty(name, VariantTypeColor4);
	if (var != nullptr && var->GetColor4() != value) {
		var->SetColor4(value);
	}
}

void MaterialInternal::SetVector4(const std::string& name, const glm::vec4& value) {
	Variant* var = GetProperty(name, VariantTypeVector4);
	if (var != nullptr && var->GetVector4() != value) {
		var->SetVector4(value);
	}
}

void MaterialInternal::SetMatrix4(const std::string& name, const glm::mat4& value) {
	Variant* var = GetProperty(name, VariantTypeMatrix4);
	if (var != nullptr /*&& var->GetMatrix4() != value*/) {
		var->SetMatrix4(value);
	}
}

void MaterialInternal::SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) {
	Variant* var = GetProperty(name, VariantTypeMatrix4Array);
	if (var != nullptr) {
		var->SetMatrix4Array(ptr, count);
	}
}

int MaterialInternal::GetInt(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeInt);
	if (var == nullptr) {
		return 0;
	}

	return var->GetInt();
}

bool MaterialInternal::GetBool(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeBool);
	if (var == nullptr) {
		return 0;
	}

	return var->GetBool();
}

float MaterialInternal::GetFloat(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeFloat);
	if (var == nullptr) {
		return 0.f;
	}

	return var->GetFloat();
}

Texture MaterialInternal::GetTexture(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeTexture);
	if (var == nullptr) {
		return nullptr;
	}

	return var->GetTexture();
}

glm::mat4 MaterialInternal::GetMatrix4(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeMatrix4);
	if (var == nullptr) {
		return glm::mat4(0);
	}

	return var->GetMatrix4();
}

glm::vec3 MaterialInternal::GetVector3(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeVector3);
	if (var == nullptr) {
		return glm::vec3(0);
	}

	return var->GetVector3();
}

glm::vec3 MaterialInternal::GetColor3(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeColor4);
	if (var == nullptr) {
		return glm::vec3(0);
	}

	return var->GetColor3();
}

glm::vec4 MaterialInternal::GetColor4(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantTypeColor4);
	if (var == nullptr) {
		return glm::vec4(0);
	}

	return var->GetColor4();
}

glm::vec4 MaterialInternal::GetVector4(const std::string& name) {
	const Variant* var = VerifyProperty(name, VariantTypeVector4);
	if (var == nullptr) {
		return glm::vec4(0);
	}

	return var->GetVector4();
}

void MaterialInternal::GetProperties(std::vector<const Property*>& properties) const {
	for (PropertyContainer::const_iterator ite = properties_.cbegin(); ite != properties_.cend(); ++ite) {
		properties.push_back(ite->second.property);
	}
}

uint MaterialInternal::GetPassCount() const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	return shader_->GetPassCount(SUB_SHADER_INDEX);
}

uint MaterialInternal::GetPassNativePointer(uint pass) const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	return shader_->GetNativePointer(SUB_SHADER_INDEX, pass);
}

void MaterialInternal::Bind(uint pass) {
	if (!IsPassEnabled(pass)) {
		Debug::LogError("pass %d is not enabled.", pass);
		return;
	}

	BindProperties(pass);
	shader_->Bind(SUB_SHADER_INDEX, pass);
	currentPass_ = pass;
}

void MaterialInternal::Unbind() {
	if (currentPass_ >= 0) {
		UnbindProperties();
		shader_->Unbind();
		currentPass_ = -1;
	}
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

int MaterialInternal::FindPass(const std::string& name) const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return -1;
	}

	return shader_->GetPassIndex(SUB_SHADER_INDEX, name);
}

void MaterialInternal::SetRenderQueue(uint value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	return shader_->SetRenderQueue(SUB_SHADER_INDEX, value);
}

uint MaterialInternal::GetRenderQueue() const {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return 0;
	}

	return shader_->GetRenderQueue(SUB_SHADER_INDEX);
}

void MaterialInternal::Define(const std::string& name) {
}

void MaterialInternal::Undefine(const std::string& name) {
}

Variant* MaterialInternal::GetProperty(const std::string& name, VariantType type) {
	PropertyContainer::iterator pos = properties_.find(name);
	if (pos == properties_.end()) {
		return nullptr;
	}

	if (pos->second.property != nullptr) {
		return &pos->second.property->value;
	}

	return nullptr;
}

Variant* MaterialInternal::VerifyProperty(const std::string& name, VariantType type) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return nullptr;
	}

	Variant* var = GetProperty(name, type);
	if (var == nullptr) {
		Debug::LogError("no property named %s.", name.c_str());
	}

	return var;
}

void MaterialInternal::BindProperties(uint pass) {
	int textureIndex = 0;
	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		if ((ite->second.mask & (1 << pass)) == 0) {
			continue;
		}

		Variant& var = ite->second.property->value;
		if (var.GetType() != VariantTypeTexture) {
			shader_->SetProperty(SUB_SHADER_INDEX, pass, ite->first, var.GetData());
		}
		else if (var.GetTexture()) {
			// TODO: for some weird reason...
 			//if (ite->first == Variables::MatrixTextureBuffer) {
 			//	textureIndex = 1;
 			//}
 			//else if (ite->first == Variables::MainTexture) {
 			//	textureIndex = 0;
 			//}
 			//else if (ite->first == Variables::ShadowDepthTexture) {
 			//	textureIndex = 2;
 			//}

			var.GetTexture()->Bind(textureIndex);
			shader_->SetProperty(SUB_SHADER_INDEX, pass, ite->first, &textureIndex);
			textureIndex++;
		}
	}
}

void MaterialInternal::UnbindProperties() {
	static float zero[sizeof(glm::mat4)  * MAX_BONE_COUNT];

	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		if ((ite->second.mask & (1 << currentPass_)) == 0) {
			continue;
		}

		Variant& var = ite->second.property->value;
		if (var.GetType() != VariantTypeTexture) {
			shader_->SetProperty(SUB_SHADER_INDEX, currentPass_, ite->first, zero);
		}
		else if (var.GetTexture()) {
			var.GetTexture()->Unbind();
		}
	}
}

void MaterialInternal::InitializeProperties() {
	std::vector<ShaderProperty> container;
	shader_->GetProperties(container);

	properties_.clear();
	for (int i = 0; i < container.size(); ++i) {
		properties_[container[i].property->name] = container[i];
	}

	Material _this = SharedThis();
	SharedTextureManager::instance()->Attach(_this);
}

void MaterialInternal::InitializeEnabledState() {
	passEnabled_ = UINT_MAX;
	for (int i = 0; i < shader_->GetPassCount(SUB_SHADER_INDEX); ++i) {
		if (!shader_->IsPassEnabled(SUB_SHADER_INDEX, i)) {
			passEnabled_ &= ~(1 << i);
		}
	}
}

void MaterialInternal::SetVariant(const std::string& name, const Variant& value) {
	switch (value.GetType()) {
		case VariantTypeInt:
			SetInt(name, value.GetInt());
			break;
		case VariantTypeFloat:
			SetFloat(name, value.GetFloat());
			break;
		case VariantTypeTexture:
			SetTexture(name, value.GetTexture());
			break;
		case VariantTypeMatrix4:
			SetMatrix4(name, value.GetMatrix4());
			break;
		case VariantTypeVector3:
			SetVector3(name, value.GetVector3());
			break;
		case VariantTypeColor3:
			SetColor3(name, value.GetColor3());
			break;
		case VariantTypeColor4:
			SetColor4(name, value.GetColor4());
			break;
		case VariantTypeVector4:
			SetVector4(name, value.GetVector4());
			break;
		default:
			Debug::LogError("invalid variant type %d.", value.GetType());
			break;
	}
}
