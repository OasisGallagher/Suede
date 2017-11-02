#include "variant.h"
#include "tools/debug.h"

std::string Variant::TypeString(VariantType type) {
	static const char* variantTypeNames[]{
		"null",
		"int",
		"bool",
		"float",
		"mat4",
		"vec3",
		"sampler",
		"quaternion",
	};

	return variantTypeNames[type];
}

int Variant::GetInt() {
	AssertX(type_ == VariantTypeInt, "invalid variant type.");
	return intValue_;
}

bool Variant::GetBool() {
	AssertX(type_ == VariantTypeBool, "invalid uniform type.");
	return boolValue_;
}

float Variant::GetFloat() {
	AssertX(type_ == VariantTypeFloat, "invalid uniform type.");
	return floatValue_;
}

glm::mat4 Variant::GetMatrix4() {
	AssertX(type_ == VariantTypeMatrix4, "invalid uniform type.");
	return mat4Value_;
}

glm::vec3 Variant::GetVector3() {
	AssertX(type_ == VariantTypeVector3, "invalid uniform type.");
	return vector3Value_;
}

glm::quat Variant::GetQuaternion() {
	AssertX(type_ == VariantTypeQuaternion, "invalid uniform type.");
	return quaternionValue_;
}

Texture Variant::GetTexture() {
	AssertX(type_ == VariantTypeTexture, "invalid uniform type.");
	return texture_;
}

int Variant::GetTextureIndex() {
	AssertX(type_ == VariantTypeTexture, "invalid uniform type.");
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
