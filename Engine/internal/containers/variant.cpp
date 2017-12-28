#include "variant.h"
#include "debug/debug.h"
#include "internal/memory/memory.h"

const char* Variant::TypeString(VariantType type) {
#define CASE(T)	case VariantType ## T: return #T
	switch (type) {
		CASE(None);
		CASE(Int);
		CASE(Bool);
		CASE(Float);
		CASE(Matrix3);
		CASE(Matrix4);
		CASE(Vector3);
		CASE(Vector4);
		CASE(Texture);
		CASE(PodBuffer);
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

const void* Variant::GetPodBuffer() const {
	if (type_ != VariantTypePodBuffer) {
		Debug::LogError("invalid uniform type.");
		return nullptr;
	}

	return data_.podBuffer.ptr;
}

uint Variant::GetPodBufferSize() const {
	if (type_ != VariantTypePodBuffer) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return data_.podBuffer.size;
}

Texture Variant::GetTexture() const {
	if (type_ != VariantTypeTexture) {
		Debug::LogError("invalid uniform type.");
		return nullptr;
	}
	return texture_;
}

int Variant::GetTextureIndex() const {
	if (type_ != VariantTypeTexture) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return data_.textureIndex;
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

void Variant::SetVector4(const glm::vec4& value) {
	SetType(VariantTypeVector4);
	data_.vec4Value = value;
}

void Variant::SetQuaternion(const glm::quat& value) {
	SetType(VariantTypeQuaternion);
	data_.quatValue = value;
}

void Variant::SetPodBuffer(const void* data, uint size) {
	if (!SetType(VariantTypePodBuffer)) {
		if (data_.podBuffer.size < size) {
			MEMORY_RELEASE_ARRAY(data_.podBuffer.ptr);
			data_.podBuffer.ptr = MEMORY_CREATE_ARRAY(char, size);
		}
	}
	else {
		data_.podBuffer.ptr = nullptr;
		if (size != 0) {
			data_.podBuffer.ptr = MEMORY_CREATE_ARRAY(char, size);
		}
	}
	
	memcpy(data_.podBuffer.ptr, data, size);
	data_.podBuffer.size = size;
}

void Variant::SetTexture(Texture value) {
	SetType(VariantTypeTexture);
	texture_ = value;
}

void Variant::SetTextureIndex(GLenum value) {
	SetType(VariantTypeTexture);
	data_.textureIndex = value;
}

const void* Variant::GetData() const {
	if (type_ == VariantTypeTexture) {
		Debug::LogError("unable to get data ptr for texture type.");
		return nullptr;
	}

	if (type_ != VariantTypePodBuffer) {
		return &data_;
	}

	return data_.podBuffer.ptr;
}

Variant& Variant::operator = (const Variant& other) {
	if (other.type_ != VariantTypePodBuffer) {
		if (type_ == VariantTypePodBuffer) {
			MEMORY_RELEASE_ARRAY(data_.podBuffer.ptr);
		}

		return *this = other;
	}

	SetPodBuffer(other.GetPodBuffer(), other.GetPodBufferSize());
	return *this;
}

bool Variant::SetType(VariantType type) {
	if (type_ == type) {
		return false;
	}

	if (type_ == VariantTypePodBuffer) {
		MEMORY_RELEASE_ARRAY(data_.podBuffer.ptr);
	}

	type_ = type;
	return true;
}

