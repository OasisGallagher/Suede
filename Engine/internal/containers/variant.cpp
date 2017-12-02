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

	return intValue_;
}

bool Variant::GetBool() {
	if (type_ != VariantTypeBool) {
		Debug::LogError("invalid uniform type.");
		return false;
	}

	return boolValue_;
}

float Variant::GetFloat() {
	if (type_ != VariantTypeFloat) {
		Debug::LogError("invalid uniform type.");
		return 0;
	}

	return floatValue_;
}

glm::mat4 Variant::GetMatrix4() {
	if (type_ != VariantTypeMatrix4) {
		Debug::LogError("invalid uniform type.");
		return glm::mat4(1);
	}

	return mat4Value_;
}

glm::vec3 Variant::GetVector3() {
	if (type_ != VariantTypeVector3) {
		Debug::LogError("invalid uniform type.");
		return glm::vec3(0);
	}

	return vector3Value_;
}

glm::vec4 Variant::GetVector4() {
	if (type_ != VariantTypeVector4) {
		Debug::LogError("invalid uniform type.");
		return glm::vec4(0, 0, 0, 1);
	}

	return vector4Value_;
}

glm::quat Variant::GetQuaternion() {
	if (type_ != VariantTypeQuaternion) {
		Debug::LogError("invalid uniform type.");
		return glm::quat();
	}

	return quaternionValue_;
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

	return textureIndex_;
}

void Variant::SetInt(int value) {
	type_ = VariantTypeInt;
	intValue_ = value;
}

void Variant::SetBool(bool value) {
	type_ = VariantTypeBool;
	boolValue_ = value;
}

void Variant::SetFloat(float value) {
	type_ = VariantTypeFloat;
	floatValue_ = value;
}

void Variant::SetMatrix4(const glm::mat4& value) {
	type_ = VariantTypeMatrix4;
	mat4Value_ = value;
}

void Variant::SetVector3(const glm::vec3& value) {
	type_ = VariantTypeVector3;
	vector3Value_ = value;
}

void Variant::SetVector4(const glm::vec4& value) {
	type_ = VariantTypeVector4;
	vector4Value_ = value;
}

void Variant::SetQuaternion(const glm::quat& value) {
	type_ = VariantTypeQuaternion;
	quaternionValue_ = value;
}

void Variant::SetTexture(Texture value) {
	type_ = VariantTypeTexture;
	texture_ = value;
}

void Variant::SetTextureLocation(GLenum value) {
	type_ = VariantTypeTexture;
	textureIndex_ = value;
}
