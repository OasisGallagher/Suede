#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "color.h"
#include "ranged.h"
#include "texture.h"

BETTER_ENUM(VariantType, int,
	None,
	Int,
	Bool,
	Float,
	RangedInt,
	RangedFloat,
	Matrix3,
	Matrix4,
	IVector3,
	Vector3,
	Color,
	Vector4,
	Texture,
	Quaternion,

	// POD array.
	_POD_ARRAY_BEGIN,
	String = _POD_ARRAY_BEGIN,
	Vector3Array,
	Matrix4Array
)

class SUEDE_API Variant {
public:
	Variant() : type_(VariantType::None) { }
	Variant(const Variant& other);
	~Variant();

public:
	int GetInt() const;
	bool GetBool() const;
	float GetFloat() const;
	iranged GetRangedInt() const;
	franged GetRangedFloat() const;
	glm::mat3 GetMatrix3() const;
	glm::mat4 GetMatrix4() const;
	glm::ivec3 GetIVector3() const;
	glm::vec3 GetVector3() const;
	Color GetColor() const;
	glm::vec4 GetVector4() const;
	glm::quat GetQuaternion() const;

	std::string GetString() const;
	
	const glm::vec3* GetVector3Array() const;
	uint GetVector3ArraySize() const;
	
	const glm::mat4* GetMatrix4Array() const;
	uint GetMatrix4ArraySize() const;
	Texture GetTexture() const;

	VariantType GetType() const { return type_; }

	void SetInt(int value);
	void SetBool(bool value);
	void SetFloat(float value);
	void SetRangedInt(const iranged& value);
	void SetRangedFloat(const franged& value);
	void SetMatrix3(const glm::mat3& value);
	void SetMatrix4(const glm::mat4& value);
	void SetIVector3(const glm::ivec3& value);
	void SetVector3(const glm::vec3& value);
	void SetColor(const Color& value);
	void SetVector4(const glm::vec4& value);
	void SetQuaternion(const glm::quat& value);
	void SetString(const std::string& value);
	void SetVector3Array(const glm::vec3* data, uint size);
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
		glm::vec2 vec2Value;
		glm::vec3 vec3Value;
		glm::vec4 vec4Value;
		glm::quat quatValue;
		Color colorValue;
		struct {
			char* ptr;
			uint size;
		} podArray;
	} data_;

	// non-pod data.
	Texture texture_;

	VariantType type_;
};
