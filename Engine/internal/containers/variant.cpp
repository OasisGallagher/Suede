#include "debug.h"
#include "variant.h"

std::string Variant::TypeString(VariantType type) {
	static const char* variantTypeNames[]{
		"null",
		"int",
		"bool",
		"float",
		"mat4",
		"vec3",
		"vec4",
		"sampler",
		"quaternion",
	};

	return variantTypeNames[type];
}

int Variant::GetInt() {
	if (type_ != VariantTypeInt) {
		Debug::LogError("invalid variant type.");
		return 0;
	}

	return data_.intValue;
}

bool Variant::GetBool() {
	if (type_ != VariantTypeBool) {
		Debug::LogError("invalid uniform type.");
		return false;
	}

	return data_.boolValue;
}

float Variant::GetFloat() {
	if (type_ != VariantTypeFloat) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return data_.floatValue;
}

glm::mat4 Variant::GetMatrix4() {
	if (type_ != VariantTypeMatrix4) {
		Debug::LogError("invalid uniform type.");
		return glm::mat4(1);
	}

	return data_.mat4Value;
}

glm::vec3 Variant::GetVector3() {
	if (type_ != VariantTypeVector3) {
		Debug::LogError("invalid uniform type.");
		return glm::vec3(0);
	}

	return data_.vector3Value;
}

glm::vec4 Variant::GetVector4() {
	if (type_ != VariantTypeVector4) {
		Debug::LogError("invalid uniform type.");
		return glm::vec4(0, 0, 0, 1);
	}

	return data_.vector4Value;
}

glm::quat Variant::GetQuaternion() {
	if (type_ != VariantTypeQuaternion) {
		Debug::LogError("invalid uniform type.");
		return glm::quat();
	}

	return data_.quaternionValue;
}

Texture Variant::GetTexture() {
	if (type_ != VariantTypeTexture) {
		Debug::LogError("invalid uniform type.");
		return nullptr;
	}
	return texture_;
}

int Variant::GetTextureIndex() {
	if (type_ != VariantTypeTexture) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return data_.textureIndex;
}

void Variant::SetInt(int value) {
	type_ = VariantTypeInt;
	data_.intValue = value;
}

void Variant::SetBool(bool value) {
	type_ = VariantTypeBool;
	data_.boolValue = value;
}

void Variant::SetFloat(float value) {
	type_ = VariantTypeFloat;
	data_.floatValue = value;
}

void Variant::SetMatrix4(const glm::mat4& value) {
	type_ = VariantTypeMatrix4;
	data_.mat4Value = value;
}

void Variant::SetVector3(const glm::vec3& value) {
	type_ = VariantTypeVector3;
	data_.vector3Value = value;
}

void Variant::SetVector4(const glm::vec4& value) {
	type_ = VariantTypeVector4;
	data_.vector4Value = value;
}

void Variant::SetQuaternion(const glm::quat& value) {
	type_ = VariantTypeQuaternion;
	data_.quaternionValue = value;
}

void Variant::SetTexture(Texture value) {
	type_ = VariantTypeTexture;
	texture_ = value;
}

void Variant::SetTextureIndex(GLenum value) {
	type_ = VariantTypeTexture;
	data_.textureIndex = value;
}
