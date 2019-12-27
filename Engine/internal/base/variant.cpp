#include "variant.h"
#include "debug/debug.h"
#include "math/mathf.h"
#include "memory/refptr.h"

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

Variant::Variant(Variant&& other) {
	if (other.type_ >= VariantType::_POD_ARRAY_BEGIN) {
		data_.podArray = other.data_.podArray;
		other.data_.podArray.ptr = nullptr;
		other.data_.podArray.size = 0;
	}
	else {
		memcpy(&data_, &other.data_, sizeof(data_));
		texture_ = other.texture_;
		type_ = other.type_;
	}

	other.type_ = VariantType::None;
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

iranged Variant::GetRangedInt() const {
	CHECK_VARIANT_TYPE(VariantType::RangedInt, iranged());
	return *(iranged*)&data_.ivec3Value;
}

franged Variant::GetRangedFloat() const {
	CHECK_VARIANT_TYPE(VariantType::RangedFloat, franged());
	return *(franged*)&data_.vec3Value;
}

Matrix4 Variant::GetMatrix4() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix4, Matrix4(1));
	return data_.mat4Value;
}

Vector3 Variant::GetVector3() const {
	CHECK_VARIANT_TYPE(VariantType::Vector3, Vector3(0));
	return data_.vec3Value;
}

IVector3 Variant::GetIVector3() const {
	CHECK_VARIANT_TYPE(VariantType::IVector3, IVector3(0));
	return data_.ivec3Value;
}

Color Variant::GetColor() const {
	CHECK_VARIANT_TYPE(VariantType::Color, Color::black);
	return data_.colorValue;
}

Vector4 Variant::GetVector4() const {
	CHECK_VARIANT_TYPE(VariantType::Vector4, Vector4(0, 0, 0, 1));
	return data_.vec4Value;
}

Quaternion Variant::GetQuaternion() const {
	CHECK_VARIANT_TYPE(VariantType::Quaternion, Quaternion());
	return data_.quatValue;
}

std::string Variant::GetString() const {
	CHECK_VARIANT_TYPE(VariantType::String, "");
	return std::string(data_.podArray.ptr, data_.podArray.ptr + data_.podArray.size);
}

const Vector3* Variant::GetVector3Array() const {
	CHECK_VARIANT_TYPE(VariantType::Vector3Array, nullptr);
	return (const Vector3*)data_.podArray.ptr;
}

const Matrix4* Variant::GetMatrix4Array() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix4Array, nullptr);
	return (const Matrix4*)data_.podArray.ptr;
}

uint Variant::GetVector3ArraySize() const {
	CHECK_VARIANT_TYPE(VariantType::Vector3Array, 0);
	return data_.podArray.size / sizeof(Vector3);
}

uint Variant::GetMatrix4ArraySize() const {
	CHECK_VARIANT_TYPE(VariantType::Matrix4Array, 0);
	return data_.podArray.size / sizeof(Matrix4);
}

Texture* Variant::GetTexture() const {
	CHECK_VARIANT_TYPE(VariantType::Texture, nullptr);
	return texture_.get();
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

void Variant::SetRangedInt(const iranged& value) {
	SetType(VariantType::RangedInt);
	memcpy(&data_.ivec3Value, &value, sizeof(value));
}

void Variant::SetRangedFloat(const franged& value) {
	SetType(VariantType::RangedFloat);
	memcpy(&data_.vec3Value, &value, sizeof(value));
}

void Variant::SetMatrix4(const Matrix4& value) {
	SetType(VariantType::Matrix4);
	data_.mat4Value = value;
}

void Variant::SetVector3(const Vector3& value) {
	SetType(VariantType::Vector3);
	data_.vec3Value = value;
}

void Variant::SetIVector3(const IVector3& value) {
	SetType(VariantType::IVector3);
	data_.ivec3Value = value;
}

void Variant::SetColor(const Color& value) {
	SetType(VariantType::Color);
	data_.colorValue = value;
}

void Variant::SetVector4(const Vector4& value) {
	SetType(VariantType::Vector4);
	data_.vec4Value = value;
}

void Variant::SetQuaternion(const Quaternion& value) {
	SetType(VariantType::Quaternion);
	data_.quatValue = value;
}

void Variant::SetString(const std::string& value) {
	SetPodArray(VariantType::String, value.c_str(), value.length());
}

void Variant::SetVector3Array(const Vector3* data, uint size) {
	SetPodArray(VariantType::Vector3Array, data, sizeof(Vector3)* size);
}

void Variant::SetMatrix4Array(const Matrix4* data, uint size) {
	SetPodArray(VariantType::Matrix4Array, data, sizeof(Matrix4) * size);
}

void Variant::SetPodArray(VariantType type, const void* data, uint size) {
	if (!SetType(type)) {
		if (data_.podArray.size < size) {
			delete[] data_.podArray.ptr;
			data_.podArray.ptr = new char[size];
		}
	}
	else {
		data_.podArray.ptr = nullptr;
		if (size != 0) {
			data_.podArray.ptr = new char[size];
		}
	}
	
	if (data != nullptr) {
		memcpy(data_.podArray.ptr, data, size);
	}

	data_.podArray.size = size;
}

#include "animation.h"
void Variant::SetTexture(Texture* value) {
	SetType(VariantType::Texture);
	texture_ = value;

	if (dynamic_cast<AnimationFrame*>(texture_.get())) {
		Debug::Break();
	}
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
		case VariantType::Matrix4: return sizeof(Matrix4);
		case VariantType::IVector3: return sizeof(IVector3);
		case VariantType::Vector3: return sizeof(Vector3);
		case VariantType::Color: return sizeof(Color);
		case VariantType::Vector4: return sizeof(Vector4);
		case VariantType::Quaternion: return sizeof(Quaternion);
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
		delete[] data_.podArray.ptr;
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
		delete[] data_.podArray.ptr;
	}

	type_ = type;
	return true;
}
