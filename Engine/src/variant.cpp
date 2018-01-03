#include "variant.h"
#include "debug/debug.h"
#include "memory/memory.h"

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
		CASE(Vector4);
		CASE(Texture);
		CASE(Matrix4Array);
		CASE(Quaternion);
	}
#undef CASE

	Debug::LogError("invalid variant type %d.", type);
	return "";
}

Variant::~Variant() {
	SetType(VariantTypeNone);
}

int Variant::GetInt() const {
	if (type_ != VariantTypeInt) {
		Debug::LogError("invalid variant type.");
		return 0;
	}

	return data_.intValue;
}

bool Variant::GetBool() const {
	if (type_ != VariantTypeBool) {
		Debug::LogError("invalid uniform type.");
		return false;
	}

	return data_.boolValue;
}

float Variant::GetFloat() const {
	if (type_ != VariantTypeFloat) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return data_.floatValue;
}

glm::mat3 Variant::GetMatrix3() const {
	if (type_ != VariantTypeMatrix3) {
		Debug::LogError("invalid uniform type.");
		return glm::mat3(1);
	}

	return data_.mat3Value;
}

glm::mat4 Variant::GetMatrix4() const {
	if (type_ != VariantTypeMatrix4) {
		Debug::LogError("invalid uniform type.");
		return glm::mat4(1);
	}

	return data_.mat4Value;
}

glm::ivec3 Variant::GetIVector3() const {
	if (type_ != VariantTypeIVector3) {
		Debug::LogError("invalid uniform type.");
		return glm::ivec3(0);
	}

	return data_.ivec3Value;
}

glm::vec3 Variant::GetVector3() const {
	if (type_ != VariantTypeVector3) {
		Debug::LogError("invalid uniform type.");
		return glm::vec3(0);
	}

	return data_.vec3Value;
}

glm::vec4 Variant::GetVector4() const {
	if (type_ != VariantTypeVector4) {
		Debug::LogError("invalid uniform type.");
		return glm::vec4(0, 0, 0, 1);
	}

	return data_.vec4Value;
}

glm::quat Variant::GetQuaternion() const {
	if (type_ != VariantTypeQuaternion) {
		Debug::LogError("invalid uniform type.");
		return glm::quat();
	}

	return data_.quatValue;
}

const glm::mat4* Variant::GetMatrix4Array() const {
	if (type_ != VariantTypeMatrix4Array) {
		Debug::LogError("invalid uniform type.");
		return nullptr;
	}

	return (const glm::mat4*)data_.podArray.ptr;
}

uint Variant::GetMatrix4ArraySize() const {
	if (type_ != VariantTypeMatrix4Array) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return data_.podArray.size / sizeof(glm::mat4);
}

Texture Variant::GetTexture() const {
	if (type_ != VariantTypeTexture) {
		Debug::LogError("invalid uniform type.");
		return nullptr;
	}

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
			MEMORY_RELEASE_ARRAY(data_.podArray.ptr);
			data_.podArray.ptr = MEMORY_CREATE_ARRAY(char, size);
		}
	}
	else {
		data_.podArray.ptr = nullptr;
		if (size != 0) {
			data_.podArray.ptr = MEMORY_CREATE_ARRAY(char, size);
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

Variant& Variant::operator = (const Variant& other) {
	if (other.type_ >= VariantTypeMatrix4Array) {
		SetPodArray(other.type_, other.data_.podArray.ptr, other.data_.podArray.size);
		return *this;
	}

	if (type_ >= VariantTypeMatrix4Array) {
		MEMORY_RELEASE_ARRAY(data_.podArray.ptr);
	}

	memcpy(&data_, &other.data_, sizeof(data_));
	texture_ = other.texture_;
	type_ = other.type_;

	return *this;
}

bool Variant::SetType(VariantType type) {
	if (type_ == type) {
		return false;
	}

	if (type_ >= VariantTypeMatrix4Array) {
		MEMORY_RELEASE_ARRAY(data_.podArray.ptr);
	}

	type_ = type;
	return true;
}

