#include "tools/math2.h"
#include "renderstate.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "renderdefines.h"
#include "materialinternal.h"
#include "builtinproperties.h"

#include "internal/rendering/sharedtexturemanager.h"

IMaterial::IMaterial() : IObject(MEMORY_NEW(MaterialInternal)) {}
void IMaterial::SetName(const std::string& value) { _suede_dptr()->SetName(value); }
std::string IMaterial::GetName() const { return _suede_dptr()->GetName(); }
void IMaterial::Bind(uint pass) { _suede_dptr()->Bind(pass); }
void IMaterial::Unbind() { _suede_dptr()->Unbind(); }
bool IMaterial::EnablePass(uint pass) { return _suede_dptr()->EnablePass(pass); }
bool IMaterial::DisablePass(uint pass) { return _suede_dptr()->DisablePass(pass); }
bool IMaterial::IsPassEnabled(uint pass) const { return _suede_dptr()->IsPassEnabled(pass); }
int IMaterial::FindPass(const std::string& name) const { return _suede_dptr()->FindPass(name); }
void IMaterial::SetPass(int pass) { _suede_dptr()->SetPass(pass); }
int IMaterial::GetPass() const { return _suede_dptr()->GetPass(); }
uint IMaterial::GetPassCount() const { return _suede_dptr()->GetPassCount(); }
uint IMaterial::GetPassNativePointer(uint pass) const { return _suede_dptr()->GetPassNativePointer(pass); }
void IMaterial::SetShader(Shader value) { _suede_dptr()->SetShader(_shared_this(), value); }
Shader IMaterial::GetShader() { return _suede_dptr()->GetShader(); }
void IMaterial::SetRenderQueue(int value) { _suede_dptr()->SetRenderQueue(value); }
int IMaterial::GetRenderQueue() const { return _suede_dptr()->GetRenderQueue(); }
void IMaterial::Define(const std::string& name) { _suede_dptr()->Define(name); }
void IMaterial::Undefine(const std::string& name) { _suede_dptr()->Undefine(name); }
void IMaterial::SetInt(const std::string& name, int value) { _suede_dptr()->SetInt(name, value); }
void IMaterial::SetBool(const std::string& name, bool value) { _suede_dptr()->SetBool(name, value); }
void IMaterial::SetFloat(const std::string& name, float value) { _suede_dptr()->SetFloat(name, value); }
void IMaterial::SetTexture(const std::string& name, Texture value) { _suede_dptr()->SetTexture(name, value); }
void IMaterial::SetMatrix4(const std::string& name, const glm::mat4& value) { _suede_dptr()->SetMatrix4(name, value); }
void IMaterial::SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) { _suede_dptr()->SetMatrix4Array(name, ptr, count); }
void IMaterial::SetVector3(const std::string& name, const glm::vec3& value) { _suede_dptr()->SetVector3(name, value); }
void IMaterial::SetVector3Array(const std::string& name, const glm::vec3* ptr, uint count) { _suede_dptr()->SetVector3Array(name, ptr, count); }
void IMaterial::SetColor(const std::string& name, const Color& value) { _suede_dptr()->SetColor(name, value); }
void IMaterial::SetVector4(const std::string& name, const glm::vec4& value) { _suede_dptr()->SetVector4(name, value); }
void IMaterial::SetVariant(const std::string& name, const Variant& value) { _suede_dptr()->SetVariant(name, value); }
int IMaterial::GetInt(const std::string& name) { return _suede_dptr()->GetInt(name); }
bool IMaterial::GetBool(const std::string& name) { return _suede_dptr()->GetBool(name); }
float IMaterial::GetFloat(const std::string& name) { return _suede_dptr()->GetFloat(name); }
iranged IMaterial::GetRangedInt(const std::string& name) { return _suede_dptr()->GetRangedInt(name); }
franged IMaterial::GetRangedFloat(const std::string& name) { return _suede_dptr()->GetRangedFloat(name); }
Texture IMaterial::GetTexture(const std::string& name) { return _suede_dptr()->GetTexture(name); }
glm::mat4 IMaterial::GetMatrix4(const std::string& name) { return _suede_dptr()->GetMatrix4(name); }
glm::vec3 IMaterial::GetVector3(const std::string& name) { return _suede_dptr()->GetVector3(name); }
Color IMaterial::GetColor(const std::string& name) { return _suede_dptr()->GetColor(name); }
glm::vec4 IMaterial::GetVector4(const std::string& name) { return _suede_dptr()->GetVector4(name); }
void IMaterial::GetProperties(std::vector<const Property*>& properties) const { _suede_dptr()->GetProperties(properties); }

// SUEDE TODO: sub shader index.
#define SUB_SHADER_INDEX	0

MaterialInternal::MaterialInternal()
	: ObjectInternal(ObjectType::Material), currentPass_(-1), name_(UNNAMED_MATERIAL) {
}

MaterialInternal::~MaterialInternal() {
}

Object MaterialInternal::Clone() {
	Material clone = NewMaterial();
	MaterialInternal* clonePtr = _suede_rptr(clone);
	*clonePtr = *this;

	return clone;
}

void MaterialInternal::SetShader(Material self, Shader value) {
	shader_ = value;
	UpdateProperties(self, value);
	InitializeEnabledState();
}

void MaterialInternal::SetInt(const std::string& name, int value) {
	Variant* var = GetProperty(name, VariantType::Int);
	if (var != nullptr) {
		if (var->GetInt() != value) { var->SetInt(value); }
	}
	else if ((var = GetProperty(name, VariantType::RangedInt)) != nullptr) {
		iranged r = var->GetRangedInt();
		if (r.get_value() != value) {
			r = value;
			var->SetRangedInt(r);
		}
	}
}

void MaterialInternal::SetBool(const std::string& name, bool value) {
	Variant* var = GetProperty(name, VariantType::Bool);
	if (var != nullptr && var->GetBool() != value) {
		var->SetBool(value);
	}
}

void MaterialInternal::SetFloat(const std::string& name, float value) {
	Variant* var = GetProperty(name, VariantType::Float);
	if (var != nullptr) {
		if (!Math::Approximately(var->GetFloat(), value)) { var->SetFloat(value); }
	}
	else if ((var = GetProperty(name, VariantType::RangedFloat)) != nullptr) {
		franged r = var->GetRangedFloat();
		if (r.get_value() != value) {
			r = value;
			var->SetRangedFloat(r);
		}
	}
}

void MaterialInternal::SetTexture(const std::string& name, Texture value) {
	Variant* var = GetProperty(name, VariantType::Texture);
	if (var != nullptr && var->GetTexture() != value) {
		var->SetTexture(value);
	}
}

void MaterialInternal::SetVector3(const std::string& name, const glm::vec3& value) {
	Variant* var = GetProperty(name, VariantType::Vector3);
	if (var != nullptr && var->GetVector3() != value) {
		var->SetVector3(value);
	}
}

void MaterialInternal::SetVector3Array(const std::string& name, const glm::vec3* ptr, uint count) {
	Variant* var = GetProperty(name, VariantType::Vector3Array);
	if (var != nullptr) {
		var->SetVector3Array(ptr, count);
	}
}

void MaterialInternal::SetColor(const std::string& name, const Color& value) {
	Variant* var = GetProperty(name, VariantType::Color);
	if (var != nullptr && var->GetColor() != value) {
		var->SetColor(value);
	}
}

void MaterialInternal::SetVector4(const std::string& name, const glm::vec4& value) {
	Variant* var = GetProperty(name, VariantType::Vector4);
	if (var != nullptr && var->GetVector4() != value) {
		var->SetVector4(value);
	}
}

void MaterialInternal::SetMatrix4(const std::string& name, const glm::mat4& value) {
	Variant* var = GetProperty(name, VariantType::Matrix4);
	if (var != nullptr /*&& var->GetMatrix4() != value*/) {
		var->SetMatrix4(value);
	}
}

void MaterialInternal::SetMatrix4Array(const std::string& name, const glm::mat4* ptr, uint count) {
	Variant* var = GetProperty(name, VariantType::Matrix4Array);
	if (var != nullptr) {
		var->SetMatrix4Array(ptr, count);
	}
}

int MaterialInternal::GetInt(const std::string& name) {
	Variant* var = GetProperty(name, VariantType::Int);
	if (var == nullptr) {
		var = GetProperty(name, VariantType::RangedInt);
	}

	if (var == nullptr) {
		Debug::LogError("no Int or RangedInt property named %s.", name.c_str());
		return 0;
	}

	return *(int*)var->GetData();
}

bool MaterialInternal::GetBool(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Bool);
	if (var == nullptr) {
		return 0;
	}

	return var->GetBool();
}

iranged MaterialInternal::GetRangedInt(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::RangedInt);
	if (var == nullptr) {
		return iranged();
	}

	return var->GetRangedInt();
}

franged MaterialInternal::GetRangedFloat(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::RangedFloat);
	if (var == nullptr) {
		return franged();
	}

	return var->GetRangedFloat();
}

float MaterialInternal::GetFloat(const std::string& name) {
	Variant* var = GetProperty(name, VariantType::Float);
	if (var == nullptr) {
		var = GetProperty(name, VariantType::RangedFloat);
	}

	if (var == nullptr) {
		Debug::LogError("no Float or RangedFloat property named %s.", name.c_str());
		return 0.f;
	}

	return *(float*)var->GetData();
}

Texture MaterialInternal::GetTexture(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Texture);
	if (var == nullptr) {
		return nullptr;
	}

	return var->GetTexture();
}

glm::mat4 MaterialInternal::GetMatrix4(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Matrix4);
	if (var == nullptr) {
		return glm::mat4(0);
	}

	return var->GetMatrix4();
}

glm::vec3 MaterialInternal::GetVector3(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Vector3);
	if (var == nullptr) {
		return glm::vec3(0);
	}

	return var->GetVector3();
}

Color MaterialInternal::GetColor(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Color);
	if (var == nullptr) {
		return Color::black;
	}

	return var->GetColor();
}

glm::vec4 MaterialInternal::GetVector4(const std::string& name) {
	const Variant* var = VerifyProperty(name, VariantType::Vector4);
	if (var == nullptr) {
		return glm::vec4(0);
	}

	return var->GetVector4();
}

void MaterialInternal::GetProperties(std::vector<const Property*>& properties) const {
	for (PropertyContainer::const_iterator ite = properties_.cbegin(); ite != properties_.cend(); ++ite) {
		if (ite->second->mask == -1) {
			properties.push_back(&ite->second->property);
		}
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

void MaterialInternal::SetRenderQueue(int value) {
	if (!shader_) {
		Debug::LogError("invalid shader");
		return;
	}

	return shader_->SetRenderQueue(SUB_SHADER_INDEX, value);
}

int MaterialInternal::GetRenderQueue() const {
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
	MaterialProperty* p = GetMaterialProperty(name, type);
	if (p != nullptr) {
		return &p->property.value;
	}

	return nullptr;
}

MaterialProperty* MaterialInternal::GetMaterialProperty(const std::string& name, VariantType type) {
	MaterialProperty* p = nullptr;
	if (properties_.get(name, p) && p != nullptr && p->property.value.GetType() == type) {
		return p;
	}

	return nullptr;
}

Variant* MaterialInternal::VerifyProperty(const std::string& name, VariantType type) {
	Variant* var = GetProperty(name, type);
	if (var == nullptr) {
		Debug::LogError("no %s property named %s.", type.to_string(), name.c_str());
	}

	return var;
}

void MaterialInternal::BindProperties(uint pass) {
	int textureIndex = 0;
	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		if ((ite->second->mask & (1 << pass)) == 0) {
			continue;
		}

		Variant& var = ite->second->property.value;
		if (var.GetType() != VariantType::Texture) {
			shader_->SetProperty(SUB_SHADER_INDEX, pass, ite->first, var.GetData());
		}
		else if (var.GetTexture()) {
			var.GetTexture()->Bind(textureIndex);
			shader_->SetProperty(SUB_SHADER_INDEX, pass, ite->first, &textureIndex);
			textureIndex++;
		}
	}
}

void MaterialInternal::UnbindProperties() {
	static float zero[sizeof(glm::mat4)  * MAX_BONE_COUNT];

	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		if ((ite->second->mask & (1 << currentPass_)) == 0) {
			continue;
		}

		Variant& var = ite->second->property.value;
		if (var.GetType() != VariantType::Texture) {
			shader_->SetProperty(SUB_SHADER_INDEX, currentPass_, ite->first, zero);
		}
		else if (var.GetTexture()) {
			var.GetTexture()->Unbind();
		}
	}
}

void MaterialInternal::UpdateProperties(Material self, Shader newShader) {
	CopyProperties(newShader);
	SharedTextureManager::instance()->Attach(self);
}

void MaterialInternal::CopyProperties(Shader newShader) {
	std::vector<ShaderProperty> shaderProperties;
	newShader->GetProperties(shaderProperties);

	// keep redundant properties in case the caller switch the previous shaders back.
	for (ShaderProperty& shaderProperty : shaderProperties) {
		const std::string& name = shaderProperty.property->name;
		MaterialProperty* materialProperty = GetMaterialProperty(name, shaderProperty.property->value.GetType());
		if (materialProperty == nullptr) {
			*properties_[name] = shaderProperty;
		}
		else {
			// copy masks and keep properties.
			materialProperty->mask = shaderProperty.mask;
		}
	}

	DeactiveRedundantProperties(shaderProperties);
}

void MaterialInternal::DeactiveRedundantProperties(const std::vector<ShaderProperty>& shaderProperties) {
	for (PropertyContainer::iterator ite = properties_.begin(); ite != properties_.end(); ++ite) {
		bool active = false;
		for (const ShaderProperty& shaderProperty : shaderProperties) {
			Property* lp = shaderProperty.property;
			Property* rp = &ite->second->property;
			if (lp->name == rp->name && lp->value.GetType() == rp->value.GetType()) {
				active = true;
				break;
			}
		}

		if (!active) {
			ite->second->mask = 0;
		}
	}
}

void MaterialInternal::InitializeEnabledState() {
	passEnabled_ = 0;
	for (int i = 0; i < shader_->GetPassCount(SUB_SHADER_INDEX); ++i) {
		if (shader_->IsPassEnabled(SUB_SHADER_INDEX, i)) {
			passEnabled_ |= (1 << i);
		}
	}
}

void MaterialInternal::SetVariant(const std::string& name, const Variant& value) {
	switch (value.GetType()) {
		case VariantType::Int:
			SetInt(name, value.GetInt());
			break;
		case VariantType::Float:
			SetFloat(name, value.GetFloat());
			break;
		case VariantType::Texture:
			SetTexture(name, value.GetTexture());
			break;
		case VariantType::Matrix4:
			SetMatrix4(name, value.GetMatrix4());
			break;
		case VariantType::Vector3:
			SetVector3(name, value.GetVector3());
			break;
		case VariantType::Color:
			SetColor(name, value.GetColor());
			break;
		case VariantType::Vector4:
			SetVector4(name, value.GetVector4());
			break;
		default:
			Debug::LogError("invalid variant type %d.", value.GetType());
			break;
	}
}

MaterialProperty& MaterialProperty::operator = (const ShaderProperty& p) {
	mask = p.mask;
	property = *p.property;
	return *this;
}
