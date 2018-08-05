#include "variant.h"
#include "debug/debug.h"
#include "tools/math2.h"
#include "memory/memory.h"

#define CHECK_VARIANT_TYPE(T, R)	if (type_ != T) { Debug::LogError("invalid uniform type."); return R; } else (void)0

const char* Variant::TypeString(VariantType type) {
#define CASE(T)	case VariantType ## T: return #T
	switch (type) {
		CASE(None);
		CASE(Int);
		CASE(Bool);
		CASE(Float);
		CASE(Matrix3);
		CASE(Matrix4);
		CASE(IVector3);
		CASE(Vector3);
		CASE(Color4);
		CASE(Vector4);
		CASE(Texture);
		CASE(Matrix4Array);
		CASE(Quaternion);
	}
#undef CASE

	Debug::LogError("invalid variant type %d.", type);
	return "";
}

Variant::Variant(const Variant& other) : Variant() {
	if (other.type_ >= VariantTypeMatrix4Array) {
		SetPodArray(other.type_, other.data_.podArray.ptr, other.data_.podArray.size);
	}
	else {
		memcpy(&data_, &other.data_, sizeof(data_));
		texture_ = other.texture_;
		type_ = other.type_;
	}
}

Variant::~Variant() {
	SetType(VariantTypeNone);
}

int Variant::GetInt() const {
	CHECK_VARIANT_TYPE(VariantTypeInt, 0);
	return data_.intValue;
}

bool Variant::GetBool() const {
	CHECK_VARIANT_TYPE(VariantTypeBool, false);
	return data_.boolValue;
}

float Variant::GetFloat() const {
	CHECK_VARIANT_TYPE(VariantTypeFloat, 0);
	return data_.floatValue;
}

glm::mat3 Variant::GetMatrix3() const {
	CHECK_VARIANT_TYPE(VariantTypeMatrix3, glm::mat3(1));
	return data_.mat3Value;
}

glm::mat4 Variant::GetMatrix4() const {
	CHECK_VARIANT_TYPE(VariantTypeMatrix4, glm::mat4(1));
	return data_.mat4Value;
}

glm::ivec3 Variant::GetIVector3() const {
	CHECK_VARIANT_TYPE(VariantTypeIVector3, glm::ivec3(0));
	return data_.ivec3Value;
}

glm::vec3 Variant::GetVector3() const {
	CHECK_VARIANT_TYPE(VariantTypeVector3, glm::vec3(0));
	return data_.vec3Value;
}

glm::vec3 Variant::GetColor3() const {
	CHECK_VARIANT_TYPE(VariantTypeColor3, glm::vec3(0));
	return data_.vec3Value;
}

glm::vec4 Variant::GetColor4() const {
	CHECK_VARIANT_TYPE(VariantTypeColor4, glm::vec4(0, 0, 0, 1));
	return data_.vec4Value;
}

glm::vec4 Variant::GetVector4() const {
	CHECK_VARIANT_TYPE(VariantTypeVector4, glm::vec4(0, 0, 0, 1));
	return data_.vec4Value;
}

glm::quat Variant::GetQuaternion() const {
	CHECK_VARIANT_TYPE(VariantTypeQuaternion, glm::quat());
	return data_.quatValue;
}

const glm::mat4* Variant::GetMatrix4Array() const {
	CHECK_VARIANT_TYPE(VariantTypeMatrix4Array, nullptr);
	return (const glm::mat4*)data_.podArray.ptr;
}

uint Variant::GetMatrix4ArraySize() const {
	CHECK_VARIANT_TYPE(VariantTypeMatrix4Array, 0);
	return data_.podArray.size / sizeof(glm::mat4);
}

Texture Variant::GetTexture() const {
	CHECK_VARIANT_TYPE(VariantTypeTexture, nullptr);
	return texture_;
}

void Variant::SetInt(int value) {
	SetType(VariantTypeInt);
	data_.intValue = value;
}

void Variant::SetBool(bool value) {
	SetType(VariantTypeBool);
	data_.boolValue = value;
}

void Variant::SetFloat(float value) {
	SetType(VariantTypeFloat);
	data_.floatValue = value;
}

void Variant::SetMatrix3(const glm::mat3& value) {
	SetType(VariantTypeMatrix3);
	data_.mat3Value = value;
}

void Variant::SetMatrix4(const glm::mat4& value) {
	SetType(VariantTypeMatrix4);
	data_.mat4Value = value;
}

void Variant::SetVector3(const glm::vec3& value) {
	SetType(VariantTypeVector3);
	data_.vec3Value = value;
}

void Variant::SetIVector3(const glm::ivec3& value) {
	SetType(VariantTypeIVector3);
	data_.ivec3Value = value;
}

void Variant::SetColor3(const glm::vec3& value) {
	SetType(VariantTypeColor3);
	data_.vec3Value = value;
}

void Variant::SetColor4(const glm::vec4& value) {
	SetType(VariantTypeColor4);
	data_.vec4Value = value;
}

void Variant::SetVector4(const glm::vec4& value) {
	SetType(VariantTypeVector4);
	data_.vec4Value = value;
}

void Variant::SetQuaternion(const glm::quat& value) {
	SetType(VariantTypeQuaternion);
	data_.quatValue = value;
}

void Variant::SetMatrix4Array(const glm::mat4* data, uint size) {
	SetPodArray(VariantTypeMatrix4Array, data, sizeof(glm::mat4) * size);
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
	
	memcpy(data_.podArray.ptr, data, size);
	data_.podArray.size = size;
}

void Variant::SetTexture(Texture value) {
	SetType(VariantTypeTexture);
	texture_ = value;
}

const void* Variant::GetData() const {
	if (type_ == VariantTypeTexture) {
		Debug::LogError("unable to get data ptr for texture type.");
		return nullptr;
	}

	if (type_ >= VariantTypeMatrix4Array) {
		return data_.podArray.ptr;
	}

	return &data_;
}

uint Variant::GetDataSize() const {
	if (type_ >= VariantTypeMatrix4Array) {
		return data_.podArray.size;
	}

	switch (type_) {
		case VariantTypeInt: return sizeof(int);
		case VariantTypeBool: return sizeof(bool);
		case VariantTypeFloat: return sizeof(float);
		case VariantTypeMatrix3: return sizeof(glm::mat3);
		case VariantTypeMatrix4: return sizeof(glm::mat4);
		case VariantTypeIVector3: return sizeof(glm::ivec3);
		case VariantTypeVector3: return sizeof(glm::vec3);
		case VariantTypeColor3: return sizeof(glm::vec3);
		case VariantTypeColor4: return sizeof(glm::vec4);
		case VariantTypeVector4: return sizeof(glm::vec4);
		case VariantTypeQuaternion: return sizeof(glm::quat);
	}

	Debug::LogError("unable to get data size for type %d.", type_);
	return 0;
}

Variant& Variant::operator = (const Variant& other) {
	if (other.type_ >= VariantTypeMatrix4Array) {
		SetPodArray(other.type_, other.data_.podArray.ptr, other.data_.podArray.size);
		return *this;
	}

	if (type_ >= VariantTypeMatrix4Array) {
		MEMORY_DELETE_ARRAY(data_.podArray.ptr);
	}

	memcpy(&data_, &other.data_, sizeof(data_));
	texture_ = other.texture_;
	type_ = other.type_;

	return *this;
}

bool Variant::operator == (const Variant& other) const {
	if (type_ != other.type_) { return false; }
	if (type_ == VariantTypeTexture) {
		return texture_ == other.texture_;
	}

	return memcmp(GetData(), other.GetData(), GetDataSize()) == 0;
}

bool Variant::SetType(VariantType type) {
	if (type_ == type) {
		return false;
	}

	if (type_ >= VariantTypeMatrix4Array) {
		MEMORY_DELETE_ARRAY(data_.podArray.ptr);
	}

	type_ = type;
	return true;
}
