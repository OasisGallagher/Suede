#include "variant.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "memory/memory.h"

#define CHECK_VARIANT_TYPE(T, R)	if (type_ != T) { Debug::LogError("invalid uniform type."); return R; } else (void)0

Variant::Variant(const Variant& other) : Variant() {
	if (other.type_ >= VariantType::_POD_ARRAY_BEGIN) {
		SetPodArray(other.type_, other.data_.podArray.ptr, other.data_.podArray.size);
	}
	else {
		memcpy(&data_, &other.data_, sizeof(data_));
		texture_ = other.texture_;
		type_ = other.type_;
	}
}

Variant::~Variant() {
	SetType(VariantType::None);
}

int Variant::GetInt() const {
	CHECK_VARIANT_TYPE(VariantType::Int, 0);
	return data_.intValue;
}

bool Variant::GetBool() const {
	CHECK_VARIANT_TYPE(VariantType::Bool, false);
	return data_.boolValue;
}

float Variant::GetFloat() const {
	CHECK_VARIANT_TYPE(VariantType::Float, 0);
	return data_.floatValue;
}

irange Variant::GetIntRange() const {
	CHECK_VARIANT_TYPE(VariantType::IntRange, irange());
	return *(irange*)&data_.ivec3Value;
}

frange Variant::GetFloatRange() const {
	CHECK_VARIANT_TYPE(VariantType::FloatRange, frange());
	return *(frange*)&data_.vec3Value;
}

glm::mat3 Variant::GetMatrix3() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix3, glm::mat3(1));
	return data_.mat3Value;
}

glm::mat4 Variant::GetMatrix4() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix4, glm::mat4(1));
	return data_.mat4Value;
}

glm::ivec3 Variant::GetIVector3() const {
	CHECK_VARIANT_TYPE(VariantType::IVector3, glm::ivec3(0));
	return data_.ivec3Value;
}

glm::vec3 Variant::GetVector3() const {
	CHECK_VARIANT_TYPE(VariantType::Vector3, glm::vec3(0));
	return data_.vec3Value;
}

Color Variant::GetColor() const {
	CHECK_VARIANT_TYPE(VariantType::Color, Color::black);
	return data_.colorValue;
}

glm::vec4 Variant::GetVector4() const {
	CHECK_VARIANT_TYPE(VariantType::Vector4, glm::vec4(0, 0, 0, 1));
	return data_.vec4Value;
}

glm::quat Variant::GetQuaternion() const {
	CHECK_VARIANT_TYPE(VariantType::Quaternion, glm::quat());
	return data_.quatValue;
}

std::string Variant::GetString() const {
	CHECK_VARIANT_TYPE(VariantType::String, "");
	return std::string(data_.podArray.ptr, data_.podArray.ptr + data_.podArray.size);
}

const glm::vec3* Variant::GetVector3Array() const {
	CHECK_VARIANT_TYPE(VariantType::Vector3Array, nullptr);
	return (const glm::vec3*)data_.podArray.ptr;
}

const glm::mat4* Variant::GetMatrix4Array() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix4Array, nullptr);
	return (const glm::mat4*)data_.podArray.ptr;
}

uint Variant::GetVector3ArraySize() const {
	CHECK_VARIANT_TYPE(VariantType::Vector3Array, 0);
	return data_.podArray.size / sizeof(glm::vec3);
}

uint Variant::GetMatrix4ArraySize() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix4Array, 0);
	return data_.podArray.size / sizeof(glm::mat4);
}

Texture Variant::GetTexture() const {
	CHECK_VARIANT_TYPE(VariantType::Texture, nullptr);
	return texture_;
}

void Variant::SetInt(int value) {
	SetType(VariantType::Int);
	data_.intValue = value;
}

void Variant::SetBool(bool value) {
	SetType(VariantType::Bool);
	data_.boolValue = value;
}

void Variant::SetFloat(float value) {
	SetType(VariantType::Float);
	data_.floatValue = value;
}

void Variant::SetIntRange(const irange& value) {
	SetType(VariantType::IntRange);
	memcpy(&data_.ivec3Value, &value, sizeof(value));
}

void Variant::SetFloatRange(const frange& value) {
	SetType(VariantType::FloatRange);
	memcpy(&data_.vec3Value, &value, sizeof(value));
}

void Variant::SetMatrix3(const glm::mat3& value) {
	SetType(VariantType::Matrix3);
	data_.mat3Value = value;
}

void Variant::SetMatrix4(const glm::mat4& value) {
	SetType(VariantType::Matrix4);
	data_.mat4Value = value;
}

void Variant::SetVector3(const glm::vec3& value) {
	SetType(VariantType::Vector3);
	data_.vec3Value = value;
}

void Variant::SetIVector3(const glm::ivec3& value) {
	SetType(VariantType::IVector3);
	data_.ivec3Value = value;
}

void Variant::SetColor(const Color& value) {
	SetType(VariantType::Color);
	data_.colorValue = value;
}

void Variant::SetVector4(const glm::vec4& value) {
	SetType(VariantType::Vector4);
	data_.vec4Value = value;
}

void Variant::SetQuaternion(const glm::quat& value) {
	SetType(VariantType::Quaternion);
	data_.quatValue = value;
}

void Variant::SetString(const std::string& value) {
	SetPodArray(VariantType::String, value.c_str(), value.length());
}

void Variant::SetVector3Array(const glm::vec3* data, uint size) {
	SetPodArray(VariantType::Vector3Array, data, sizeof(glm::vec3)* size);
}

void Variant::SetMatrix4Array(const glm::mat4* data, uint size) {
	SetPodArray(VariantType::Matrix4Array, data, sizeof(glm::mat4) * size);
}

void Variant::SetPodArray(VariantType type, const void* data, uint size) {
	if (!SetType(type)) {
		if (data_.podArray.size < size) {
			MEMORY_DELETE_ARRAY(data_.podArray.ptr);
			data_.podArray.ptr = MEMORY_NEW_ARRAY(char, size);
		}
	}
	else {
		data_.podArray.ptr = nullptr;
		if (size != 0) {
			data_.podArray.ptr = MEMORY_NEW_ARRAY(char, size);
		}
	}
	
	if (data != nullptr) {
		memcpy(data_.podArray.ptr, data, size);
	}

	data_.podArray.size = size;
}

void Variant::SetTexture(Texture value) {
	SetType(VariantType::Texture);
	texture_ = value;
}

const void* Variant::GetData() const {
	if (type_ == VariantType::Texture) {
		Debug::LogError("unable to get data ptr for texture type.");
		return nullptr;
	}

	if (type_ >= VariantType::_POD_ARRAY_BEGIN) {
		return data_.podArray.ptr;
	}

	return &data_;
}

uint Variant::GetDataSize() const {
	if (type_ >= VariantType::_POD_ARRAY_BEGIN) {
		return data_.podArray.size;
	}

	switch (type_) {
		case VariantType::Int: return sizeof(int);
		case VariantType::Bool: return sizeof(bool);
		case VariantType::Float: return sizeof(float);
		case VariantType::Matrix3: return sizeof(glm::mat3);
		case VariantType::Matrix4: return sizeof(glm::mat4);
		case VariantType::IVector3: return sizeof(glm::ivec3);
		case VariantType::Vector3: return sizeof(glm::vec3);
		case VariantType::Color: return sizeof(Color);
		case VariantType::Vector4: return sizeof(glm::vec4);
		case VariantType::Quaternion: return sizeof(glm::quat);
	}

	Debug::LogError("unable to get data size for type %d.", type_);
	return 0;
}

Variant& Variant::operator = (const Variant& other) {
	if (other.type_ >= VariantType::_POD_ARRAY_BEGIN) {
		SetPodArray(other.type_, other.data_.podArray.ptr, other.data_.podArray.size);
		return *this;
	}

	if (type_ >= VariantType::_POD_ARRAY_BEGIN) {
		MEMORY_DELETE_ARRAY(data_.podArray.ptr);
	}

	memcpy(&data_, &other.data_, sizeof(data_));
	texture_ = other.texture_;
	type_ = other.type_;

	return *this;
}

bool Variant::operator == (const Variant& other) const {
	if (type_ != other.type_) { return false; }
	if (type_ == VariantType::Texture) {
		return texture_ == other.texture_;
	}

	return memcmp(GetData(), other.GetData(), GetDataSize()) == 0;
}

bool Variant::SetType(VariantType type) {
	if (type_ == type) {
		return false;
	}

	if (type_ >= VariantType::_POD_ARRAY_BEGIN) {
		MEMORY_DELETE_ARRAY(data_.podArray.ptr);
	}

	type_ = type;
	return true;
}
