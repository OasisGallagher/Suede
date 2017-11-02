#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "texture.h"

enum VariantType {
	VariantTypeNone,
	VariantTypeInt,
	VariantTypeBool,
	VariantTypeFloat,
	VariantTypeMatrix4,
	VariantTypeVector3,
	VariantTypeTexture,
	VariantTypeQuaternion,
};

class Variant {
public:
	Variant() :type_(VariantTypeNone) {}

public:
	static std::string TypeString(VariantType type);

public:
	int GetInt();
	bool GetBool();
	float GetFloat();
	glm::mat4 GetMatrix4();
	glm::vec3 GetVector3();
	glm::quat GetQuaternion();
	Texture GetTexture();
	int GetTextureIndex();

	VariantType GetType() { return type_; }

	void SetInt(int value);
	void SetBool(bool value);
	void SetFloat(float value);
	void SetMatrix4(const glm::mat4& value);
	void SetVector3(const glm::vec3& value);
	void SetQuaternion(const glm::quat& value);
	void SetTexture(Texture value);
	void SetTextureLocation(GLenum value);

private:
	union { // pod only.
		int intValue_;
		bool boolValue_;
		float floatValue_;
		glm::mat4 mat4Value_;
		glm::vec3 vector3Value_;
		int textureIndex_;
		glm::quat quaternionValue_;
	};

	// non-pod data.
	Texture texture_;

	VariantType type_;
};
