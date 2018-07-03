#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "texture.h"

enum VariantType {
	VariantTypeNone,
	VariantTypeInt,
	VariantTypeBool,
	VariantTypeFloat,
	VariantTypeMatrix3,
	VariantTypeMatrix4,
	VariantTypeIVector3,
	VariantTypeVector3,
	VariantTypeColor3,
	VariantTypeColor4,
	VariantTypeVector4,
	VariantTypeTexture,
	VariantTypeQuaternion,

	// Buffer type.
	VariantTypeMatrix4Array,
};

class SUEDE_API Variant {
public:
	Variant() :type_(VariantTypeNone) { }
	Variant(const Variant& other);

	~Variant();

public:
	static const char* TypeString(VariantType type);

public:
	int GetInt() const;
	bool GetBool() const;
	float GetFloat() const;
	glm::mat3 GetMatrix3() const;
	glm::mat4 GetMatrix4() const;
	glm::ivec3 GetIVector3() const;
	glm::vec3 GetVector3() const;
	glm::vec3 GetColor3() const;
	glm::vec4 GetColor4() const;
	glm::vec4 GetVector4() const;
	glm::quat GetQuaternion() const;
	const glm::mat4* GetMatrix4Array() const;
	uint GetMatrix4ArraySize() const;
	Texture GetTexture() const;

	VariantType GetType() const { return type_; }

	void SetInt(int value);
	void SetBool(bool value);
	void SetFloat(float value);
	void SetMatrix3(const glm::mat3& value);
	void SetMatrix4(const glm::mat4& value);
	void SetIVector3(const glm::ivec3& value);
	void SetVector3(const glm::vec3& value);
	void SetColor3(const glm::vec3& value);
	void SetColor4(const glm::vec4& value);
	void SetVector4(const glm::vec4& value);
	void SetQuaternion(const glm::quat& value);
	void SetMatrix4Array(const glm::mat4* data, uint size);
	void SetTexture(Texture value);

	const void* GetData() const;
	uint GetDataSize() const;

public:
	Variant& operator = (const Variant& other);
	bool operator == (const Variant& other) const;

private:
	bool SetType(VariantType type);
	void SetPodArray(VariantType type, const void* data, uint size);

private:
	union Data { // pod only.
		Data() {}

		int intValue;
		bool boolValue;
		float floatValue;
		glm::mat3 mat3Value;
		glm::mat4 mat4Value;
		glm::ivec3 ivec3Value;
		glm::vec3 vec3Value;
		glm::vec4 vec4Value;
		glm::quat quatValue;

		struct {
			char* ptr;
			uint size;
		} podArray;
	} data_;

	// non-pod data.
	Texture texture_;

	VariantType type_;
};
