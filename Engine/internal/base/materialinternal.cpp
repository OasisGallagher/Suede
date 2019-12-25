#include "materialinternal.h"

#include "context.h"
#include "renderstate.h"
#include "math/mathf.h"
#include "debug/debug.h"
#include "tools/string.h"
#include "renderdefines.h"
#include "builtinproperties.h"

Material::Material() : Object(new MaterialInternal(Context::GetCurrent())) { }
ref_ptr<Object> Material::Clone() { return _suede_dptr()->Clone(); }
void Material::Bind(uint pass) { _suede_dptr()->Bind(pass); }
void Material::Unbind() { _suede_dptr()->Unbind(); }
bool Material::EnablePass(uint pass) { return _suede_dptr()->EnablePass(pass); }
bool Material::DisablePass(uint pass) { return _suede_dptr()->DisablePass(pass); }
bool Material::IsPassEnabled(uint pass) const { return _suede_dptr()->IsPassEnabled(pass); }
int Material::FindPass(const std::string& name) const { return _suede_dptr()->FindPass(name); }
void Material::SetPass(int pass) { _suede_dptr()->SetPass(pass); }
int Material::GetPass() const { return _suede_dptr()->GetPass(); }
uint Material::GetPassCount() const { return _suede_dptr()->GetPassCount(); }
uint Material::GetPassNativePointer(uint pass) const { return _suede_dptr()->GetPassNativePointer(pass); }
void Material::SetShader(Shader* value) { _suede_dptr()->SetShader(this, value); }
Shader* Material::GetShader() { return _suede_dptr()->GetShader(); }
void Material::SetRenderQueue(int value) { _suede_dptr()->SetRenderQueue(value); }
int Material::GetRenderQueue() const { return _suede_dptr()->GetRenderQueue(); }
void Material::Define(const std::string& name) { _suede_dptr()->Define(name); }
void Material::Undefine(const std::string& name) { _suede_dptr()->Undefine(name); }
void Material::SetInt(const std::string& name, int value) { _suede_dptr()->SetInt(name, value); }
bool Material::HasProperty(const std::string& name) const { return _suede_dptr()->HasProperty(name); }
void Material::SetBool(const std::string& name, bool value) { _suede_dptr()->SetBool(name, value); }
void Material::SetFloat(const std::string& name, float value) { _suede_dptr()->SetFloat(name, value); }
void Material::SetTexture(const std::string& name, Texture* value) { _suede_dptr()->SetTexture(name, value); }
void Material::SetMatrix4(const std::string& name, const Matrix4& value) { _suede_dptr()->SetMatrix4(name, value); }
void Material::SetMatrix4Array(const std::string& name, const Matrix4* ptr, uint count) { _suede_dptr()->SetMatrix4Array(name, ptr, count); }
void Material::SetVector3(const std::string& name, const Vector3& value) { _suede_dptr()->SetVector3(name, value); }
void Material::SetVector3Array(const std::string& name, const Vector3* ptr, uint count) { _suede_dptr()->SetVector3Array(name, ptr, count); }
void Material::SetColor(const std::string& name, const Color& value) { _suede_dptr()->SetColor(name, value); }
void Material::SetVector4(const std::string& name, const Vector4& value) { _suede_dptr()->SetVector4(name, value); }
void Material::SetVariant(const std::string& name, const Variant& value) { _suede_dptr()->SetVariant(name, value); }
int Material::GetInt(const std::string& name) { return _suede_dptr()->GetInt(name); }
bool Material::GetBool(const std::string& name) { return _suede_dptr()->GetBool(name); }
float Material::GetFloat(const std::string& name) { return _suede_dptr()->GetFloat(name); }
iranged Material::GetRangedInt(const std::string& name) { return _suede_dptr()->GetRangedInt(name); }
franged Material::GetRangedFloat(const std::string& name) { return _suede_dptr()->GetRangedFloat(name); }
Texture* Material::GetTexture(const std::string& name) { return _suede_dptr()->GetTexture(name); }
Matrix4 Material::GetMatrix4(const std::string& name) { return _suede_dptr()->GetMatrix4(name); }
Vector3 Material::GetVector3(const std::string& name) { return _suede_dptr()->GetVector3(name); }
Color Material::GetColor(const std::string& name) { return _suede_dptr()->GetColor(name); }
Vector4 Material::GetVector4(const std::string& name) { return _suede_dptr()->GetVector4(name); }
const std::vector<const Property*>& Material::GetExplicitProperties() { return _suede_dptr()->GetExplicitProperties(); }

// SUEDE TODO: sub shader index.
#define SUB_SHADER_INDEX	0

event<Material*> MaterialInternal::shaderChanged;

MaterialInternal::MaterialInternal(Context* context)
	: ObjectInternal(ObjectType::Material), context_(context), currentPass_(-1) {
	name_ = "New Material";
	context_->destroyed.subscribe(this, &MaterialInternal::OnContextDestroyed);
}

MaterialInternal::~MaterialInternal() {
	if (context_ != nullptr) {
		context_->destroyed.unsubscribe(this);
	}
}

ref_ptr<Object> MaterialInternal::Clone() {
	Material* clone = new Material();
	MaterialInternal* clonePtr = _suede_rptr(clone);
	*clonePtr = *this;

	return clone;
}

void MaterialInternal::SetShader(Material* self, Shader* value) {
	shader_ = value;
	CopyProperties(value);
	InitializeEnabledState();

	shaderChanged.raise(self);
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
		if (!Mathf::Approximately(var->GetFloat(), value)) { var->SetFloat(value); }
	}
	else if ((var = GetProperty(name, VariantType::RangedFloat)) != nullptr) {
		franged r = var->GetRangedFloat();
		if (r.get_value() != value) {
			r = value;
			var->SetRangedFloat(r);
		}
	}
}

void MaterialInternal::SetTexture(const std::string& name, Texture* value) {
	Variant* var = GetProperty(name, VariantType::Texture);
	if (var != nullptr && var->GetTexture() != value) {
		var->SetTexture(value);
	}
}

void MaterialInternal::SetVector3(const std::string& name, const Vector3& value) {
	Variant* var = GetProperty(name, VariantType::Vector3);
	if (var != nullptr && var->GetVector3() != value) {
		var->SetVector3(value);
	}
}

void MaterialInternal::SetVector3Array(const std::string& name, const Vector3* ptr, uint count) {
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

void MaterialInternal::SetVector4(const std::string& name, const Vector4& value) {
	Variant* var = GetProperty(name, VariantType::Vector4);
	if (var != nullptr && var->GetVector4() != value) {
		var->SetVector4(value);
	}
}

void MaterialInternal::SetMatrix4(const std::string& name, const Matrix4& value) {
	Variant* var = GetProperty(name, VariantType::Matrix4);
	if (var != nullptr /*&& var->GetMatrix4() != value*/) {
		var->SetMatrix4(value);
	}
}

void MaterialInternal::SetMatrix4Array(const std::string& name, const Matrix4* ptr, uint count) {
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

Texture* MaterialInternal::GetTexture(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Texture);
	if (var == nullptr) {
		return nullptr;
	}

	return var->GetTexture();
}

Matrix4 MaterialInternal::GetMatrix4(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Matrix4);
	if (var == nullptr) {
		return Matrix4(0);
	}

	return var->GetMatrix4();
}

Vector3 MaterialInternal::GetVector3(const std::string& name) {
	Variant* var = VerifyProperty(name, VariantType::Vector3);
	if (var == nullptr) {
		return Vector3(0);
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

Vector4 MaterialInternal::GetVector4(const std::string& name) {
	const Variant* var = VerifyProperty(name, VariantType::Vector4);
	if (var == nullptr) {
		return Vector4(0);
	}

	return var->GetVector4();
}

const std::vector<const Property*>& MaterialInternal::GetExplicitProperties() {
	return explicitProperties_;
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

void MaterialInternal::OnContextDestroyed() {
	properties_.clear();
	explicitProperties_.clear();

	context_ = nullptr;
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
	static float zero[sizeof(Matrix4)  * MAX_BONE_COUNT];

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

void MaterialInternal::CopyProperties(Shader* newShader) {
	std::vector<ShaderProperty> shaderProperties;
	newShader->GetProperties(shaderProperties);

	explicitProperties_.clear();

	// keep redundant properties in case the caller switch the previous shaders back.
	for (int i = 0; i < shaderProperties.size(); ++i) {
		ShaderProperty& shaderProperty = shaderProperties[i];
		const std::string& name = shaderProperty.property->name;
		MaterialProperty* materialProperty = GetMaterialProperty(name, shaderProperty.property->value.GetType());
		if (materialProperty == nullptr) {
			materialProperty = properties_[name];

			materialProperty->i = i;
			materialProperty->mask = shaderProperty.mask;
			materialProperty->property = *shaderProperty.property;
		}
		else {
			// copy masks and keep properties.
			materialProperty->i = i;
			materialProperty->mask = shaderProperty.mask;
		}

		if (shaderProperty.mask == -1) {
			explicitProperties_.push_back(&materialProperty->property);
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
